#include "map_renderer.h"


using namespace renderer;

using std::literals::string_literals::operator""s;

bool detail::SphereProjector::IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point detail::SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

const Settings& MapRenderer::GetSettngs () const {
    return settings_;
}

MapRenderer& MapRenderer::SetBorder(double width, double height, double padding) {
    settings_.width = std::move(width);
    settings_.height = std::move(height);
    settings_.padding = std::move(padding);
    return *this;
}

MapRenderer& MapRenderer::SetLineWidth(double width) {
    settings_.stroke_width = std::move(width);
    return *this;
}

MapRenderer& MapRenderer::SetStopRadius(double radius) {
    settings_.stop_radius = std::move(radius);
    return *this;
}

MapRenderer& MapRenderer::SetBusFont(double font_size, double font_offset_x, double font_offset_y) {
    settings_.bus_lable_font_size = std::move(font_size);
    settings_.bus_lable_offset = {std::move(font_offset_x), std::move(font_offset_y)};
    return *this;
}

MapRenderer& MapRenderer::SetStopFont(double font_size, double font_offset_x, double font_offset_y) {
    settings_.stop_lable_font_size = std::move(font_size);
    settings_.stop_lable_offset = {std::move(font_offset_x), std::move(font_offset_y)};
    return *this;
}

MapRenderer& MapRenderer::SetUnderlayerWidth(double width) {
    settings_.underlayer_width = std::move(width);
    return *this;
}

svg::Color MapRenderer::MakeColor(const std::string& str)  const {
    return (str);
}

svg::Color MapRenderer::MakeColor(int R, int G, int B)  const {
    return (svg::Rgb(R,G,B));
}

svg::Color MapRenderer::MakeColor(int R, int G, int B, double opacity)  const {
    return (svg::Rgba(R,G,B, opacity));
}

MapRenderer& MapRenderer::SetUnderlayerColor(svg::Color color) {
    settings_.underlayer_color = std::move(color);
    return *this;
}

MapRenderer& MapRenderer::AddColorInPalette(svg::Color color) {
    settings_.color_palette.push_back(std::move(color));
    return *this;
}

svg::Polyline MapRenderer::RenderBusRoute (const t_catalogue::Bus* bus_It, const detail::SphereProjector& function, size_t num_palette) const{
    svg::Polyline bus_route;
    bus_route.SetFillColor("none"s)
            .SetStrokeColor(settings_.color_palette[num_palette])
            .SetStrokeWidth(settings_.stroke_width)
            .SetStrokeLineCap(settings_.stroke_linecap)
            .SetStrokeLineJoin(settings_.stroke_linejoin);

    for(const auto& stop : bus_It->route)
        bus_route.AddPoint(function(stop->coordinates));

    return bus_route;
}
svg::Text MapRenderer::RenderDefaultUnderlayer(const svg::Point coordinates, const std::string& name) const{
    return svg::Text().SetFontFamily("Verdana"s)
                    .SetPosition(coordinates)
                    .SetFillColor(settings_.underlayer_color)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineCap(settings_.stroke_linecap)
                    .SetStrokeLineJoin(settings_.stroke_linejoin)
                    .SetData(name);
}

svg::Text MapRenderer::RenderBusUnderlayer (const svg::Point coordinates, const std::string& name) const{
    svg::Text underlayer = RenderDefaultUnderlayer(coordinates, name);
    return underlayer.SetOffset(settings_.bus_lable_offset)
                    .SetFontSize(settings_.bus_lable_font_size)
                    .SetFontWeight("bold"s);
}

svg::Text MapRenderer::RenderStopUnderlayer (const svg::Point coordinates, const std::string& name) const{
    svg::Text underlayer = RenderDefaultUnderlayer(coordinates, name);
    return underlayer.SetOffset(settings_.stop_lable_offset)
                    .SetFontSize(settings_.stop_lable_font_size);
}

svg::Text MapRenderer::RenderDefaultName(const svg::Point coordinates, const std::string& name, svg::Color color) const {
    return svg::Text().SetPosition(coordinates)
                    .SetFontFamily("Verdana"s)
                    .SetData(name)
                    .SetFillColor(color);
}

svg::Text MapRenderer::RenderBusName (const svg::Point coordinates, const std::string& name, size_t num_palette) const{
    svg::Text bus_name = RenderDefaultName(coordinates, name, settings_.color_palette[num_palette]);
    return bus_name.SetOffset(settings_.bus_lable_offset)
                    .SetFontSize(settings_.bus_lable_font_size)
                    .SetFontWeight("bold"s);
}

svg::Text MapRenderer::RenderStopName (const svg::Point coordinates, const std::string& name) const{
    svg::Text stop_name = RenderDefaultName(coordinates, name, "black"s);
    return stop_name.SetOffset(settings_.stop_lable_offset)
                    .SetFontSize(settings_.stop_lable_font_size);
}

svg::Circle MapRenderer::RenderStop (const svg::Point coordinates) const{
    return svg::Circle().SetCenter(coordinates)
                        .SetRadius(settings_.stop_radius)
                        .SetFillColor("white"s);
}

svg::Document MapRenderer::RenderMap (const std::vector<t_catalogue::Bus*>buses, const std::vector<t_catalogue::Stop*> map) const {
    const detail::SphereProjector proj(map.begin(), map.end(),
                                    settings_.width,
                                    settings_.height,
                                    settings_.padding);

    svg::Document document;
    for (auto bus_It = buses.begin(); bus_It != buses.end(); ++bus_It)
        document.Add(RenderBusRoute(*bus_It, proj, (bus_It - buses.begin())%settings_.color_palette.size()) );

    for (auto bus_It = buses.begin(); bus_It != buses.end(); ++bus_It) {

        const auto coordinates = proj( (*bus_It)->route.front()->coordinates);
        document.Add( RenderBusUnderlayer( coordinates, (*bus_It)->name) );
        document.Add( RenderBusName( coordinates, (*bus_It)->name, (bus_It - buses.begin())%settings_.color_palette.size() ) );

        if (!(*bus_It)->is_round && (*bus_It)->route[ (*bus_It)->route.size()/2 ]->name != (*bus_It)->route.front()->name ) {
            const auto coordinates = proj((*bus_It)->route[(*bus_It)->route.size()/2]->coordinates);
            document.Add( RenderBusUnderlayer( coordinates, (*bus_It)->name ) );
            document.Add( RenderBusName( coordinates, (*bus_It)->name, (bus_It - buses.begin())%settings_.color_palette.size()));
        }

    }

    for(const auto& stop : map)
        document.Add( RenderStop( proj(stop->coordinates) ) );

    for(const auto& stop : map) {
        document.Add( RenderStopUnderlayer( proj(stop->coordinates), stop->name));
        document.Add( RenderStopName (proj(stop->coordinates), stop->name));
    }

        return document;
    }

