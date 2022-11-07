#include "serialization.h"

using namespace serialization;

db::RouterSettings Serializator::SerializationRoutSettings () const{
    db::RouterSettings rout_settings;
    rout_settings.set_bus_wait_time(router_->GetWaitTime());
    rout_settings.set_bus_velocity(router_->GetVelocity());
    return rout_settings;
}

db::MapSettings Serializator::SerializationMapSettings () const {
    db::MapSettings map_settings;
    const auto settings = render_->GetSettngs();

    map_settings.set_width(settings.width);
    map_settings.set_height(settings.height);
    map_settings.set_padding(settings.padding);
    map_settings.set_stroke_width(settings.stroke_width);
    map_settings.set_stop_radius(settings.stop_radius);
    map_settings.set_bus_lable_font_size(settings.bus_lable_font_size);
    map_settings.set_stop_lable_font_size(settings.stop_lable_font_size);
    map_settings.set_underlayer_width(settings.underlayer_width);

    db::Point bus_lable_offset, stop_lable_offset;
    bus_lable_offset.set_x(settings.bus_lable_offset.x);
    bus_lable_offset.set_y(settings.bus_lable_offset.y);
    *map_settings.mutable_bus_lable_offset() = std::move(bus_lable_offset);

    stop_lable_offset.set_x(settings.stop_lable_offset.x);
    stop_lable_offset.set_y(settings.stop_lable_offset.y);
    *map_settings.mutable_stop_lable_offset() = std::move(stop_lable_offset);

    db::Color underlayer_color;
    if (const auto* pval = std::get_if<std::string>(&settings.underlayer_color))
        underlayer_color.set_s_color(*pval);
    else if (const auto* pval = std::get_if<svg::Rgb>(&settings.underlayer_color)) {
        db::Rgb rgb;
        rgb.set_red(pval->red);
        rgb.set_green(pval->green);
        rgb.set_blue(pval->blue);

        *underlayer_color.mutable_rgb_color() = std::move(rgb);
    }else if (const auto* pval = std::get_if<svg::Rgba>(&settings.underlayer_color)) {
        db::Rgba rgba;
        rgba.set_red(pval->red);
        rgba.set_green(pval->green);
        rgba.set_blue(pval->blue);
        rgba.set_opacity(pval->opacity);

        *underlayer_color.mutable_rgba_color() = std::move(rgba);
    }

    *map_settings.mutable_underlayer_color() = std::move(underlayer_color);

    for (const auto color : settings.color_palette) {
        db::Color p_color;
        if (const auto* pval = std::get_if<std::string>(&color))
            p_color.set_s_color(*pval);
        else if (const auto* pval = std::get_if<svg::Rgb>(&color)) {
            db::Rgb rgb;
            rgb.set_red(pval->red);
            rgb.set_green(pval->green);
            rgb.set_blue(pval->blue);

            *p_color.mutable_rgb_color() = std::move(rgb);
        }else if (const auto* pval = std::get_if<svg::Rgba>(&color)) {
            db::Rgba rgba;
            rgba.set_red(pval->red);
            rgba.set_green(pval->green);
            rgba.set_blue(pval->blue);
            rgba.set_opacity(pval->opacity);

            *p_color.mutable_rgba_color() = std::move(rgba);
        }
        *map_settings.add_color_palette() = std::move(p_color);
    }

    return map_settings;
}

Serializator& Serializator::SetPath(const Path& path) {
        path_to_bd_ = path;
        return *this;
}

void Serializator::SaveData () {

    db::TransportCatalogue database;
    std::unordered_map<const t_catalogue::Stop*,uint32_t> stop_to_num;

    for (const t_catalogue::Stop* ptn_stop : catalogue_->GetAllStops()){
        db::Stop stop;
        stop.set_name (ptn_stop->name);
        stop.set_lat (ptn_stop->coordinates.lat);
        stop.set_lng (ptn_stop->coordinates.lng);
        *database.add_stops() = std::move(stop);
        stop_to_num[ptn_stop] = stop_to_num.size();
    }

    for (const auto [stop_to_stop, distance] : catalogue_->GetAllStopToStopDistance()){
        db::StopDistance stop_distance;
        stop_distance.set_stop_a_id(stop_to_num.at(stop_to_stop.first));
        stop_distance.set_stop_b_id(stop_to_num.at(stop_to_stop.second));
        stop_distance.set_distance(distance);

        *database.add_stop_distance() = std::move(stop_distance);
    }

    for (const t_catalogue::Bus* ptn_bus : catalogue_->GetAllBus()){
        db::Bus bus;
        bus.set_name (ptn_bus->name);
        for (const t_catalogue::Stop* stop : ptn_bus->route)
            bus.add_stop_id( stop_to_num.at(stop));

        bus.set_is_round(ptn_bus->is_round);

        *database.add_buses() = std::move(bus);
    }

    *database.mutable_map_settings() = SerializationMapSettings();

    *database.mutable_router_settings() = SerializationRoutSettings();

    std::ofstream output (path_to_bd_, std::ios::binary);
    database.SerializeToOstream(&output);
}


void Serializator::LoadData () {
    std::ifstream input (path_to_bd_, std::ios::binary);
    db::TransportCatalogue database;
    database.ParseFromIstream(&input);

    std::vector<std::string_view> stop_name;
    stop_name.reserve(database.stops_size());
    for (int i = 0, max = database.stops_size(); i < max; ++i ) {
        catalogue_->AddStop(database.stops(i).name(),database.stops(i).lat(),database.stops(i).lng());
        stop_name.push_back(database.stops(i).name());
    }

    for (int i = 0, max = database.stop_distance_size(); i < max; ++i )
        catalogue_->AddDistance(stop_name[database.stop_distance(i).stop_a_id()],
                                stop_name[database.stop_distance(i).stop_b_id()],
                                database.stop_distance(i).distance());

    for (int i = 0, max = database.buses_size(); i < max; ++i ) {
        std::vector<std::string_view> stop_array;
        stop_array.reserve(database.buses(i).stop_id_size());
        for (int j = 0, max_j = database.buses(i).stop_id_size(); j < max_j; ++j )
            stop_array.push_back(stop_name[database.buses(i).stop_id(j)]);

        catalogue_->AddBus(database.buses(i).name(), stop_array, database.buses(i).is_round());
    }

    render_->SetBorder(database.map_settings().width(),
                       database.map_settings().height(),
                       database.map_settings().padding()
                        )
            .SetLineWidth(database.map_settings().stroke_width())
            .SetStopRadius(database.map_settings().stop_radius())
            .SetBusFont(database.map_settings().bus_lable_font_size(),
                        database.map_settings().bus_lable_offset().x(),
                        database.map_settings().bus_lable_offset().y()
                        )
            .SetStopFont(database.map_settings().stop_lable_font_size(),
                        database.map_settings().stop_lable_offset().x(),
                        database.map_settings().stop_lable_offset().y()
                        )
            .SetUnderlayerWidth(database.map_settings().underlayer_width());

    if (database.map_settings().underlayer_color().has_rgba_color())
        render_->SetUnderlayerColor(render_->MakeColor(database.map_settings().underlayer_color().rgba_color().red(),
                                                            database.map_settings().underlayer_color().rgba_color().green(),
                                                            database.map_settings().underlayer_color().rgba_color().blue(),
                                                            database.map_settings().underlayer_color().rgba_color().opacity()
                                                            )
                                        );
    else if (database.map_settings().underlayer_color().has_rgb_color())
        render_->SetUnderlayerColor(render_->MakeColor(database.map_settings().underlayer_color().rgb_color().red(),
                                                            database.map_settings().underlayer_color().rgb_color().green(),
                                                            database.map_settings().underlayer_color().rgb_color().blue()
                                                            )
                                        );
    else render_->SetUnderlayerColor(render_->MakeColor(database.map_settings().underlayer_color().s_color()));

    for (int i = 0, max = database.map_settings().color_palette_size(); i < max; ++i) {
        if (database.map_settings().color_palette(i).has_rgba_color())
            render_->AddColorInPalette(render_->MakeColor(database.map_settings().color_palette(i).rgba_color().red(),
                                                            database.map_settings().color_palette(i).rgba_color().green(),
                                                            database.map_settings().color_palette(i).rgba_color().blue(),
                                                            database.map_settings().color_palette(i).rgba_color().opacity()
                                                            )
                                        );
        else if (database.map_settings().color_palette(i).has_rgb_color())
            render_->AddColorInPalette(render_->MakeColor(database.map_settings().color_palette(i).rgb_color().red(),
                                                            database.map_settings().color_palette(i).rgb_color().green(),
                                                            database.map_settings().color_palette(i).rgb_color().blue()
                                                            )
                                        );
        else render_->AddColorInPalette(render_->MakeColor(database.map_settings().color_palette(i).s_color()));

    }

    router_->SetWaitTime(database.router_settings().bus_wait_time());
    router_->SetVelocity(database.router_settings().bus_velocity());

}
