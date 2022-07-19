#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

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

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

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

//request_handler::RequestHandler* const handler_;

public:
//    MapRenderer (request_handler::RequestHandler* const handler) : handler_(handler) {
//    }

    MapRenderer& SetBorder(double width, double height, double padding) {
        settings_.width = std::move(width);
        settings_.height = std::move(height);
        settings_.padding = std::move(padding);
        return *this;
    }

    MapRenderer& SetLineWidth(double width) {
        settings_.stroke_width = std::move(width);
        return *this;
    }

    MapRenderer& SetStopRadius(double radius) {
        settings_.stop_radius = std::move(radius);
        return *this;
    }

    MapRenderer& SetBusFont(double font_size, double font_offset_x, double font_offset_y) {
        settings_.bus_lable_font_size = std::move(font_size);
        settings_.bus_lable_offset = {std::move(font_offset_x), std::move(font_offset_y)};
        return *this;
    }

    MapRenderer& SetStopFont(double font_size, double font_offset_x, double font_offset_y) {
        settings_.stop_lable_font_size = std::move(font_size);
        settings_.stop_lable_offset = {std::move(font_offset_x), std::move(font_offset_y)};
        return *this;
    }

    MapRenderer& SetUnderlayerWidth(double width) {
        settings_.underlayer_width = std::move(width);
        return *this;
    }

    svg::Color MakeColor(int R, int G, int B)  const {
        return (svg::Rgb(R,G,B));
    }

    svg::Color MakeColor(int R, int G, int B, double opacity)  const {
        return (svg::Rgba(R,G,B, opacity));
    }

    MapRenderer& SetUnderlayerColor(svg::Color color) {
        settings_.underlayer_color = std::move(color);
        return *this;
    }

    MapRenderer& SetAddColorInPalette(svg::Color color) {
        settings_.color_palette.push_back(std::move(color));
        return *this;
    }

    svg::Document RenderMap (const std::vector<t_catalogue::Bus*> map_) const {
        for (auto bus : map_) {

        }
    };

};

}
