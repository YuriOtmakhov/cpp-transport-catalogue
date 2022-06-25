#include "json_reader.h"

#include <list>

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
        catalogue_->AddBus(bus_info.at("name"s).AsString(), ParsQueryRoute(bus_info.at("stops"s).AsArray(), bus_info.at("is_roundtrip"s).AsBool() ));
    }
}

json::Node JsonReader::StopRequests(const json::Node request) const {
    json::Array buses;
    for(const auto& bus : handler_->GetBusesByStop(request.AsMap().at("name"s).AsString()))
        buses.push_back(bus->name);
    return json::Dict({{"request_id"s, request.AsMap().at("id"s).AsInt()},{"buses"s, buses}});
}

json::Node JsonReader::BusRequests(const json::Node request) const {
    transport_catalogue::BusDate bus_date = handler_->GetBusInfo(request.AsMap().at("name"s).AsString());

    return json::Dict({{"request_id"s, request.AsMap().at("id"s).AsInt()},
                        {"curvature"s, bus_date.curvature},
                        {"route_length"s, static_cast<int>(bus_date.length)},
                        {"stop_count"s, static_cast<int>(bus_date.stops)},
                        {"unique_stop_count"s, static_cast<int>(bus_date.unique_stops)}});

}

const json::Document JsonReader::ParsingStatRequests(const json::Node& document) const {
    json::Array ans_array;
    if (document.AsArray().empty())
        return json::Document(json::Node(ans_array));

    for (const auto& request : document.AsArray()) {
        try {
            if (request.AsMap().at("type"s).AsString() == "Stop"s)
                ans_array.push_back(StopRequests(request));
            else
                ans_array.push_back(BusRequests(request));
        }
            catch(std::out_of_range&) {
                ans_array.push_back(json::Dict({{"request_id"s, request.AsMap().at("id"s).AsInt()},
                        {"error_message"s, "not found"s}}));
            }
    }
    return json::Document(json::Node(ans_array));
}

void JsonReader::ReadJSON (std::istream& input) {
    json_document_ = json::Load(input);

    ParsingBaseRequests( (*json_document_).GetRoot().AsMap().at("base_requests"s));

}

void JsonReader::WriteJSON (std::ostream& output) const {
    if (json_document_)
        json::Print(ParsingStatRequests((*json_document_).GetRoot().AsMap().at("stat_requests"s)), output);

}


