#include "svg.h"

namespace svg {

using namespace std::literals;

Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {
}

Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double o) : Rgb(r,g,b), opacity(o) {
}

Point::Point(double x, double y): x(x), y(y) {
}

RenderContext::RenderContext(std::ostream& out): out(out) {
}

RenderContext::RenderContext(std::ostream& out, int indent_step, int indent)
    : out(out)
    , indent_step(indent_step)
    , indent(indent) {
}

RenderContext RenderContext::Indented() const {
    return {out, indent_step, indent + indent_step};
}

void RenderContext::RenderIndent() const {
    for (int i = 0; i < indent; ++i) {
        out.put(' ');
    }
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    context.out <<' ';
    RenderObject(context);

    context.out << std::endl;
}

std::ostream& operator<< (std::ostream& out,const StrokeLineCap line_cap) {
    using namespace std::literals;
    switch (line_cap) {
        case StrokeLineCap::BUTT: out<<"butt"s; break;
        case StrokeLineCap::ROUND: out<<"round"s; break;
        case StrokeLineCap::SQUARE: out<<"square"s; break;
    }
    return out;
}

struct ColorPrinter {
    std::ostream& out;

    ColorPrinter (std::ostream& output = std::cout) : out(output) {
    }
    void operator() (std::monostate) {
        using namespace std::literals;
        out<<"none"s;
    }
    void operator() (std::string str) {
        out<<str;
    }
    void operator() (svg::Rgb color) {
        using namespace std::literals;
        out<<"rgb("s<< +color.red <<','<<+color.green<<','<<+color.blue<<")"s;
    }
    void operator() (svg::Rgba color) {
        using namespace std::literals;
        out<<"rgba("s<<+color.red<<','<<+color.green<<','<<+color.blue<<','<<color.opacity<<")"s;
    }
};

std::ostream& operator<< (std::ostream& out,const StrokeLineJoin line_join) {
    using namespace std::literals;
    switch (line_join) {
        case StrokeLineJoin::ARCS: out<<"arcs"s; break;
        case StrokeLineJoin::BEVEL: out<<"bevel"s; break;
        case StrokeLineJoin::MITER: out<<"miter"s; break;
        case StrokeLineJoin::MITER_CLIP: out<<"miter-clip"s; break;
        case StrokeLineJoin::ROUND: out<<"round"s; break;
    }
    return out;
}

std::ostream& operator<< (std::ostream& out, const Color color) {
    using namespace std::literals;
    std::visit(ColorPrinter{out}, color);
    return out;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;

    bool flag = false;

    for (const auto& point : points_)
        if (flag)
            out<<' '<<point.x<<','<<point.y;
        else {
            flag = true;
            out<<point.x<<','<<point.y;
        }
//        out<<point.x<<','<<point.y<<' ';
    out<< "\" ";
    RenderAttrs(out);
    out<<"/>"sv;
}

std::string Text::print (const char ch) const{
    switch (ch) {
        case '"': return "&quot;"s;
        case '<': return "&lt;"s;
        case '>': return "&gt;"s;
        case '&': return "&amp;"s;
        case '\'': return "&apos;"s;
        }
    return ""s;
}

void Text::RenderObject(const RenderContext& context) const{
    auto& out = context.out;
    out << "<text x=\""sv<< position_.x<<"\" y=\""sv << position_.y
    <<"\" dx=\""sv<<offset_.x<< "\" dy=\""sv<< offset_.y<< "\" font-size=\""sv << size_<<"\"";
    if (!font_family_.empty())
        out<<" font-family=\""sv<<font_family_<<"\"";
    if (!font_weight_.empty())
        out<<" font-weight=\""sv<<font_weight_<<"\"";
    RenderAttrs(out);
    out<<">"sv;
    for(const char ch: data_)
        if (!print(ch).empty())
            out<<print(ch);
        else
            out<<ch;
    out<< "</text>"sv;
}

Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family){
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
//objects_.push_back(std::make_shared<Object>(obj));
}

void Document::Render(std::ostream& out) const {
    out<<"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv
    <<"<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    for (const auto& object : objects_)
        object->Render(out);
    out<<"</svg>\n";
}

}  // namespace svg
