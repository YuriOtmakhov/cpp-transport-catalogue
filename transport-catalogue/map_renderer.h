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
            [](auto lhs, auto rhs) { return lhs->coordinates.lng < rhs->coordinates.lng; });
        min_lon_ = (*left_it)->coordinates.lng;
        const double max_lon = (*right_it)->coordinates.lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs->coordinates.lat < rhs->coordinates.lat; });
        const double min_lat = (*bottom_it)->coordinates.lat;
        max_lat_ = (*top_it)->coordinates.lat;

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

    svg::Document RenderMap (const std::vector<t_catalogue::Bus*>buses, const std::vector<t_catalogue::Stop*> map) const {
        const detail::SphereProjector proj(map.begin(), map.end(),
                                    settings_.width,
                                    settings_.height,
                                    settings_.padding);

        svg::Document document;
//        size_t num_color = 0;
        for (auto bus_It = buses.begin(); bus_It != buses.end(); ++bus_It) {
            svg::Polyline bus_route;
            bus_route.SetFillColor("none")
                .SetStrokeColor(settings_.color_palette[(bus_It - buses.begin())% settings_.color_palette.size()])
                .SetStrokeWidth(settings_.stroke_width)
                .SetStrokeLineCap(settings_.stroke_linecap)
                .SetStrokeLineJoin(settings_.stroke_linejoin);

            for(const auto& stop : (*bus_It)->route)
                bus_route.AddPoint(proj(stop->coordinates));

            document.Add(std::move(bus_route));
        }
            //-------------------------------------------------------------
        for (auto bus_It = buses.begin(); bus_It != buses.end(); ++bus_It) {
            svg::Text bus_name, underlayer;
            bus_name.SetPosition(proj((*bus_It)->route.front()->coordinates))
                    .SetOffset(settings_.bus_lable_offset)
                    .SetFontSize(settings_.bus_lable_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetData((*bus_It)->name);
            underlayer = bus_name;

            bus_name.SetFillColor(settings_.color_palette[(bus_It - buses.begin())% settings_.color_palette.size()]);

            underlayer.SetFillColor(settings_.underlayer_color)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineCap(settings_.stroke_linecap)
                    .SetStrokeLineJoin(settings_.stroke_linejoin);

            document.Add(underlayer);
            document.Add(bus_name);
            if (!(*bus_It)->is_round && (*bus_It)->route[(*bus_It)->route.size()/2]->name!= (*bus_It)->route.front()->name ) {
                svg::Text bus_name_end = bus_name, underlayer_end = underlayer;
                bus_name_end.SetPosition(proj((*bus_It)->route[(*bus_It)->route.size()/2]->coordinates));
                underlayer_end.SetPosition(proj((*bus_It)->route[(*bus_It)->route.size()/2]->coordinates));
                document.Add(underlayer_end);
                document.Add(bus_name_end);
            }

            //-------------------------------------------------------------
        }

        for(const auto& stop : map)
            document.Add(svg::Circle().SetCenter(proj(stop->coordinates))
                                    .SetRadius(settings_.stop_radius)
                                    .SetFillColor("white")
                        );

        //-------------------------------------------------------------
        for(const auto& stop : map) {
            svg::Text stop_name, underlayer;
            stop_name.SetPosition(proj(stop->coordinates))
                    .SetOffset(settings_.stop_lable_offset)
                    .SetFontSize(settings_.stop_lable_font_size)
                    .SetFontFamily("Verdana")
//                    .SetFontWeight("bold")
                    .SetData(stop->name);
            underlayer = stop_name;

            stop_name.SetFillColor("black");

            underlayer.SetFillColor(settings_.underlayer_color)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineCap(settings_.stroke_linecap)
                    .SetStrokeLineJoin(settings_.stroke_linejoin);

            document.Add(underlayer);
            document.Add(stop_name);
        }

        return document;
    };

};

}
