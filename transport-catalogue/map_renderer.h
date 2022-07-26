#pragma once

#include <algorithm>
//#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <string>

#include "request_handler.h"
#include "svg.h"
#include "domain.h"
#include "geo.h"

namespace renderer {

namespace detail {

class SphereProjector {
static inline const double EPSILON = 1e-6;

double padding_;
double min_lon_ = 0;
double max_lat_ = 0;
double zoom_coeff_ = 0;

bool IsZero(double value);

public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding);

    svg::Point operator()(geo::Coordinates coords) const;

};

}

class MapRenderer {

struct {
    double width;
    double height;
    double padding;
    double stroke_width;
    svg::StrokeLineCap stroke_linecap = svg::StrokeLineCap::ROUND;
    svg::StrokeLineJoin stroke_linejoin = svg::StrokeLineJoin::ROUND;
    double stop_radius;
    double bus_lable_font_size;
    svg::Point bus_lable_offset;
    double stop_lable_font_size;
    svg::Point stop_lable_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
} settings_;

svg::Polyline RenderBusRoute (const t_catalogue::Bus* bus_It, const detail::SphereProjector& function, size_t num_palette) const;
svg::Text RenderDefaultUnderlayer(const svg::Point coordinates, const std::string& name) const;
svg::Text RenderBusUnderlayer (const svg::Point coordinates, const std::string& name) const;
svg::Text RenderStopUnderlayer (const svg::Point coordinates, const std::string& name) const;
svg::Text RenderDefaultName(const svg::Point coordinates, const std::string& name, svg::Color color) const;
svg::Text RenderBusName (const svg::Point coordinates, const std::string& name, size_t num_palette) const;
svg::Text RenderStopName (const svg::Point coordinates, const std::string& name) const;
svg::Circle RenderStop (const svg::Point coordinates) const;

public:

    MapRenderer& SetBorder(double width, double height, double padding);

    MapRenderer& SetLineWidth(double width);

    MapRenderer& SetStopRadius(double radius);

    MapRenderer& SetBusFont(double font_size, double font_offset_x, double font_offset_y);

    MapRenderer& SetStopFont(double font_size, double font_offset_x, double font_offset_y);

    MapRenderer& SetUnderlayerWidth(double width);

    svg::Color MakeColor(const std::string& str)  const;

    svg::Color MakeColor(int R, int G, int B)  const;

    svg::Color MakeColor(int R, int G, int B, double opacity)  const;

    MapRenderer& SetUnderlayerColor(svg::Color color);

    MapRenderer& AddColorInPalette(svg::Color color);

    svg::Document RenderMap (const std::vector<t_catalogue::Bus*>buses, const std::vector<t_catalogue::Stop*> map) const;

};

namespace detail {

template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                                double max_width, double max_height, double padding): padding_(padding){

    if (points_begin == points_end)
        return;

    const auto [left_it, right_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs->coordinates.lng < rhs->coordinates.lng; });
    min_lon_ = (*left_it)->coordinates.lng;
    const double max_lon = (*right_it)->coordinates.lng;

    const auto [bottom_it, top_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs->coordinates.lat < rhs->coordinates.lat; });
    const double min_lat = (*bottom_it)->coordinates.lat;
    max_lat_ = (*top_it)->coordinates.lat;

    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_))
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);

    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat))
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);

    if (width_zoom && height_zoom)
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    else if (width_zoom)
        zoom_coeff_ = *width_zoom;
    else if (height_zoom)
        zoom_coeff_ = *height_zoom;
}
}

}
