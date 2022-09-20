#include "json_reader.h"

#include <list>
#include <sstream>

using namespace json_reader;
using std::literals::string_literals::operator""s;

std::vector<std::string_view> JsonReader::ParsQueryRoute (const json::Array stops, bool is_round) const {
    std::vector<std::string_view> route;
    route.reserve( is_round ? stops.size() : (2*stops.size()) );

    for(const auto& stop : stops)
        route.push_back(stop.AsString());

    if (!is_round)
        for (int i = route.size()-2; i>=0 ; --i)
            route.push_back(route[i]);
    return route;
}

void JsonReader::ParsingBaseRequests (const json::Node& document) {
    if (document.AsArray().empty())
        return;
    std::list<const json::Node*> request_queue;
    for (const auto& request : document.AsArray())
        if (request.AsMap().at("type"s).AsString() == "Stop"s)
            request_queue.push_front(&request);
        else
            request_queue.push_back(&request);

    auto It_query = request_queue.begin();

    std::list<std::pair<std::string_view, std::list<std::pair<std::string_view, size_t>>>> distance_to_stop;

    while ( (*It_query)->AsMap().at("type"s).AsString() == "Stop"s) {
        const json::Dict& stop_info = (*It_query++)->AsMap();

        catalogue_->AddStop( stop_info.at("name"s).AsString(),
                            stop_info.at("latitude"s).AsDouble(),
                            stop_info.at("longitude"s).AsDouble()
                                );

        std::list<std::pair<std::string_view, size_t>> stop_list;
        for (auto& stop : stop_info.at("road_distances"s).AsMap())
            stop_list.push_back({stop.first, stop.second.AsInt()});

        distance_to_stop.push_back({stop_info.at("name"s).AsString(), std::move(stop_list)});

    }

    for (const auto& distance : distance_to_stop)
        catalogue_->AddDistance(distance.first, distance.second);

    while ( It_query != request_queue.end()) {
        const auto& bus_info = (*(It_query++))->AsMap();
        catalogue_->AddBus(bus_info.at("name"s).AsString(), ParsQueryRoute(bus_info.at("stops"s).AsArray(), bus_info.at("is_roundtrip"s).AsBool() ), bus_info.at("is_roundtrip"s).AsBool());
    }
}

json::Node JsonReader::StopRequests(const json::Node request) const {
    json::Array buses;
    for(const auto& bus : handler_->GetBusesByStop(request.AsMap().at("name"s).AsString()))
        buses.push_back(bus->name);
    return json::Builder{}.StartDict()
            .Key("request_id"s).Value(request.AsMap().at("id"s).AsInt())
            .Key("buses"s).Value(buses)
            .EndDict().Build();
//    return json::Dict({{"request_id"s, request.AsMap().at("id"s).AsInt()},{"buses"s, buses}});
}

json::Node JsonReader::BusRequests(const json::Node request) const {
    t_catalogue::BusDate bus_date = handler_->GetBusInfo(request.AsMap().at("name"s).AsString());

    return json::Builder{}.StartDict()
            .Key("request_id"s).Value(request.AsMap().at("id"s).AsInt())
            .Key("curvature"s).Value(bus_date.curvature)
            .Key("route_length"s).Value(static_cast<int>(bus_date.length))
            .Key("stop_count"s).Value(static_cast<int>(bus_date.stops))
            .Key("unique_stop_count"s).Value(static_cast<int>(bus_date.unique_stops))
            .EndDict().Build();
//    return json::Dict({{"request_id"s, request.AsMap().at("id"s).AsInt()},
//                        {"curvature"s, bus_date.curvature},
//                        {"route_length"s, static_cast<int>(bus_date.length)},
//                        {"stop_count"s, static_cast<int>(bus_date.stops)},
//                        {"unique_stop_count"s, static_cast<int>(bus_date.unique_stops)}});

}

json::Node JsonReader::MapRequests(const json::Node request) const {
    std::ostringstream map;
    render_->RenderMap( handler_->GetAllRound(), handler_->GetMap()).Render(map);
    return json::Builder{}.StartDict()
            .Key("request_id"s).Value(request.AsMap().at("id"s).AsInt())
            .Key("map"s).Value(map.str())
            .EndDict().Build();
//    return json::Dict({{"request_id"s, request.AsMap().at("id"s).AsInt()},
//                        {"map"s, map.str()}});

}

json::Node JsonReader::RouteRequests(const json::Node request) const {
    if(!router_->IsInit())
        router_->InitRouter(handler_->GetAllRound(), catalogue_->GetAllStops());

    auto [total_time, round_items]= router_->BuilRoute(catalogue_->FindStop(request.AsMap().at("from"s).AsString()),catalogue_->FindStop(request.AsMap().at("to"s).AsString()));
    json::Array items;
    for (const auto& item : round_items)
        if (item.span_count)
            items.push_back( json::Builder{}.StartDict()
                            .Key("type"s).Value("Bus"s)
                            .Key("bus"s).Value(static_cast<std::string>(item.name))
                            .Key("span_count"s).Value(item.span_count)
                            .Key("time"s).Value(item.time)
                            .EndDict().Build() );
        else
           items.push_back( json::Builder{}.StartDict()
                            .Key("type"s).Value("Wait"s)
                            .Key("stop_name"s).Value(static_cast<std::string>(item.name))
                            .Key("time"s).Value(item.time)
                            .EndDict().Build() );

    return json::Builder{}.StartDict()
            .Key("request_id"s).Value(request.AsMap().at("id"s).AsInt())
            .Key("total_time"s).Value(total_time)
            .Key("items"s).Value(items)
            .EndDict().Build();
}

const json::Document JsonReader::ParsingStatRequests(const json::Node& document) const {
    json::Array ans_array;
    if (document.AsArray().empty())
        return json::Document(json::Node(ans_array));

    for (const auto& request : document.AsArray()) {
        try {
            if (request.AsMap().at("type"s).AsString() == "Stop"s)
                ans_array.push_back(StopRequests(request));
            else if (request.AsMap().at("type"s).AsString() == "Bus"s)
                ans_array.push_back(BusRequests(request));
            else if (request.AsMap().at("type"s).AsString() == "Map"s)
                ans_array.push_back(MapRequests(request));
            else if (request.AsMap().at("type"s).AsString() == "Route"s)
                ans_array.push_back(RouteRequests(request));
        }
            catch(std::out_of_range&) {
                ans_array.push_back(json::Builder{}.StartDict()
                                            .Key("request_id"s).Value(request.AsMap().at("id"s).AsInt())
                                            .Key("error_message"s).Value("not found"s)
                                    .EndDict().Build());
//                ans_array.push_back(json::Dict({{"request_id"s, request.AsMap().at("id"s).AsInt()},
//                        {"error_message"s, "not found"s}}));
            }
    }
    return json::Document(json::Node(ans_array));
}

auto JsonReader::ParsingColor (const json::Node& color) const{

    if (color.IsString())
        return render_->MakeColor(color.AsString());
    else if (color.AsArray().size() == 3)
        return (render_->MakeColor(color.AsArray()[0].AsInt(),
                color.AsArray()[1].AsInt(),
                color.AsArray()[2].AsInt()
                ));
    else
       return (render_->MakeColor(color.AsArray()[0].AsInt(),
                color.AsArray()[1].AsInt(),
                color.AsArray()[2].AsInt(),
                color.AsArray()[3].AsDouble()
                ));
}

void JsonReader::ParsingRenderSettings (const json::Node& settings) {
    render_->SetBorder(settings.AsMap().at("width"s).AsDouble(),
                        settings.AsMap().at("height"s).AsDouble(),
                        settings.AsMap().at("padding"s).AsDouble()
                        )
            .SetLineWidth(settings.AsMap().at("line_width"s).AsDouble())
            .SetStopRadius(settings.AsMap().at("stop_radius"s).AsDouble())
            .SetBusFont(settings.AsMap().at("bus_label_font_size"s).AsDouble(),
                        settings.AsMap().at("bus_label_offset"s).AsArray().front().AsDouble(),
                        settings.AsMap().at("bus_label_offset"s).AsArray().back().AsDouble()
                        )
            .SetStopFont(settings.AsMap().at("stop_label_font_size"s).AsDouble(),
                        settings.AsMap().at("stop_label_offset"s).AsArray().front().AsDouble(),
                        settings.AsMap().at("stop_label_offset"s).AsArray().back().AsDouble()
                        )
            .SetUnderlayerWidth(settings.AsMap().at("underlayer_width"s).AsDouble());

    render_->SetUnderlayerColor(ParsingColor(settings.AsMap().at("underlayer_color"s)));

    for (const auto& color : settings.AsMap().at("color_palette"s).AsArray())
        render_->AddColorInPalette(ParsingColor(color));

}

void JsonReader::ParsingRoutingSettings(const json::Node& settings) {
    router_->SetWaitTime(settings.AsMap().at("bus_wait_time"s).AsInt())
            .SetVelocity(settings.AsMap().at("bus_velocity"s).AsDouble());
}

void JsonReader::ReadJSON (std::istream& input) {
    json_document_ = json::Load(input);

    ParsingBaseRequests( (*json_document_).GetRoot().AsMap().at("base_requests"s) );

    ParsingRenderSettings( (*json_document_).GetRoot().AsMap().at("render_settings"s) );

    ParsingRoutingSettings( (*json_document_).GetRoot().AsMap().at("routing_settings"s) );

}

void JsonReader::WriteJSON (std::ostream& output) const {
    if (json_document_)
        json::Print(ParsingStatRequests((*json_document_).GetRoot().AsMap().at("stat_requests"s)), output);

}


