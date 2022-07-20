#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <list>
#include <optional>
#include <variant>

namespace svg {

struct Rgb {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    Rgb() = default;
    Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {
    };
};

struct Rgba : public Rgb {
    double opacity = 1.0;
    Rgba() = default;
    Rgba(uint8_t r, uint8_t g, uint8_t b, double o) : Rgb(r,g,b), opacity(o) {
    };
};

using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

inline const Color NoneColor{};

std::ostream& operator<< (std::ostream& out, const Color color);

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE
};

std::ostream& operator<< (std::ostream& out,const StrokeLineCap line_cap);

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND
};

std::ostream& operator<< (std::ostream& out,const StrokeLineJoin line_join);

template <class Owner>
class PathProps {

std::optional<Color> fill_color_;
std::optional<Color> stroke_color_;
std::optional<double> stroke_width_;
std::optional<StrokeLineCap> line_cap_;
std::optional<StrokeLineJoin> line_join_;

public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return static_cast<Owner&>(*this);
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return static_cast<Owner&>(*this);
    }
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = std::move(width);
        return static_cast<Owner&>(*this);
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = std::move(line_cap);
        return static_cast<Owner&>(*this);
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = std::move(line_join);
        return static_cast<Owner&>(*this);
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_)
            out << " fill=\""sv << *fill_color_ << "\" "sv;
        if (stroke_color_)
            out << " stroke=\""sv << *stroke_color_ << "\" "sv;
        if (stroke_width_)
            out << " stroke-width=\""sv<< *stroke_width_ << "\" "sv;
        if (line_cap_)
            out << " stroke-linecap=\""sv << *line_cap_ <<"\" "sv;
        if (line_join_)
            out << " stroke-linejoin=\""sv << *line_join_ <<"\" "sv;
    }
};

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */

class Object {

virtual void RenderObject(const RenderContext& context) const = 0;

public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;
};

class ObjectContainer {
public:
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    template <typename Obj>
    void Add(Obj obj) {
        AddPtr(std::make_unique<Obj>(std::move(obj)));
    }
protected:
    ~ObjectContainer() = default;
};

class Drawable {
public:
    virtual void Draw (ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {

Point center_;
double radius_ = 1.0;

void RenderObject(const RenderContext& context) const override;

public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final: public Object, public PathProps<Polyline> {

std::list<Point> points_;

void RenderObject(const RenderContext& context) const override;

public:
    Polyline& AddPoint(Point point);
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final: public Object, public PathProps<Text> {

Point position_;
Point offset_;
uint32_t size_ = 1;
std::string font_family_;
std::string font_weight_;
std::string data_;

std::string print (const char ch) const;

void RenderObject(const RenderContext& context) const override;

public:
    Text& SetPosition(Point pos);
    Text& SetOffset(Point offset);
    Text& SetFontSize(uint32_t size);
    Text& SetFontFamily(std::string font_family);
    Text& SetFontWeight(std::string font_weight);
    Text& SetData(std::string data);
};

class Document: public ObjectContainer {
std::list<std::unique_ptr<Object>> objects_;
//std::list<std::shared_ptr<Object>> objects_;
public:
    /*
     Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
     Пример использования:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */
    template <typename Obj>
    void Add(Obj obj) {
        objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
    }
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document
};

}  // namespace svg
