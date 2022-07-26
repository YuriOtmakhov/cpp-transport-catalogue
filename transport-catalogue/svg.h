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
    Rgb(uint8_t r, uint8_t g, uint8_t b);
};

struct Rgba : public Rgb {
    double opacity = 1.0;
    Rgba() = default;
    Rgba(uint8_t r, uint8_t g, uint8_t b, double o);
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
    Owner& SetFillColor(Color color);
    Owner& SetStrokeColor(Color color);
    Owner& SetStrokeWidth(double width);
    Owner& SetStrokeLineCap(StrokeLineCap line_cap);
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join);

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const;
};

struct Point {
    Point() = default;
    Point(double x, double y);
    double x = 0;
    double y = 0;
};

struct RenderContext {
    RenderContext(std::ostream& out);

    RenderContext(std::ostream& out, int indent_step, int indent = 0);

    RenderContext Indented() const;

    void RenderIndent() const;

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

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

public:

    template <typename Obj>
    void Add(Obj obj);

    void AddPtr(std::unique_ptr<Object>&& obj) override;

    void Render(std::ostream& out) const;

};

//-----------------
template <class Owner>
Owner& PathProps<Owner>::SetFillColor(Color color) {
    fill_color_ = std::move(color);
    return static_cast<Owner&>(*this);
}

template <class Owner>
Owner& PathProps<Owner>::SetStrokeColor(Color color) {
    stroke_color_ = std::move(color);
    return static_cast<Owner&>(*this);
}

template <class Owner>
Owner& PathProps<Owner>::SetStrokeWidth(double width) {
    stroke_width_ = std::move(width);
    return static_cast<Owner&>(*this);
}
template <class Owner>
Owner& PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap) {
    line_cap_ = std::move(line_cap);
    return static_cast<Owner&>(*this);
}

template <class Owner>
Owner& PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join) {
    line_join_ = std::move(line_join);
    return static_cast<Owner&>(*this);
}

template <class Owner>
void PathProps<Owner>::RenderAttrs(std::ostream& out) const {
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

template <typename Obj>
void Document::Add(Obj obj) {
    objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
}
//---------------

}  // namespace svg
