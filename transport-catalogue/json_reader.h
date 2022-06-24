#pragma once

#include <iostream>
#include <list>
#include <optional>

#include "request_handler.h"
#include "transport_catalogue.h"
#include "json.h"

namespace json_reader {

class JsonReader {

transport_catalogue::TransportCatalogue* const catalogue_;
request_handler::RequestHandler* const handler_;
std::optional<json::Document> json_document_;

std::vector<std::string_view> ParsQueryRoute (const json::Array stops, bool is_round) {
    std::vector<std::string_view> route;// = {"Morskoy vokzal", "Rivierskiy most"};
    route.reserve( is_round ? stops.size() : (2*stops.size()) );

    for(const auto& stop : stops)
        route.push_back(stop.AsString());

    if (!is_round)
        for (int i = route.size()-2; i>=0 ; --i)
            route.push_back(route[i]);
    return route;
}

void ParsingBaseRequests (const json::Node& document) {
    using std::literals::string_literals::operator""s;
//    if (document.IsNull())
//        return;

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

//    for (const auto& distance : distance_to_stop) {
//        std::cerr << distance.first<< std::endl;
//        for (const auto& item : distance.second)
//            std::cerr << item.first << " -c- "<< item.second << std::endl;
//    }

    for (const auto& distance : distance_to_stop)
        catalogue_->AddDistance(distance.first, distance.second);

    while ( It_query != request_queue.end()) {
        const auto& bus_info = (*(It_query++))->AsMap();
        catalogue_->AddBus(bus_info.at("name"s).AsString(), ParsQueryRoute(bus_info.at("stops"s).AsArray(), bus_info.at("is_roundtrip"s).AsBool() ));
    }
}

json::Node StopRequests(const json::Node request) {
    using std::literals::string_literals::operator""s;
    json::Array buses;
    for(const auto& bus : handler_->GetBusesByStop(request.AsMap().at("name"s).AsString()))
        buses.push_back(bus->name);
//    if (buses.empty())
//        buses.push_back("no buses"s);
//        return json::Dict({{"request_id"s, request.AsMap().at("id"s).AsInt()},{"buses"s, "no buses"s}});
    return json::Dict({{"request_id"s, request.AsMap().at("id"s).AsInt()},{"buses"s, buses}});
}

json::Node BusRequests(const json::Node request) {
    using std::literals::string_literals::operator""s;
    transport_catalogue::BusDate bus_date = handler_->GetBusInfo(request.AsMap().at("name"s).AsString());

    return json::Dict({{"request_id"s, request.AsMap().at("id"s).AsInt()},
                        {"curvature"s, bus_date.curvature},
                        {"route_length"s, static_cast<int>(bus_date.length)},
                        {"stop_count"s, static_cast<int>(bus_date.stops)},
                        {"unique_stop_count"s, static_cast<int>(bus_date.unique_stops)}});

}

const json::Document ParsingStatRequests(const json::Node& document) {
    using std::literals::string_literals::operator""s;
    json::Array ans_array;
//    if (!document.IsNull())
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

public:
    explicit JsonReader(request_handler::RequestHandler* const handler) : catalogue_(handler->GetTransportCatalogue ()),handler_(handler) {
    };

    void ReadJSON (std::istream& input) {
        using std::literals::string_literals::operator""s;
//        if (!json_document_)
//            std::cerr<<"KO0"s<<std::endl;
        try{
        json_document_ = json::Load(input);
        }
        catch(...) {
            std::cout<<"[]"<<std::endl;
            return;
        }
        if ((*json_document_).GetRoot().AsMap().at("base_requests"s).IsNull())
        {
            std::cout<<"[]"<<std::endl;
            return;
        }
//        if ((*json_document_).GetRoot().IsMap())
//            std::cerr<<"KO1"s<<std::endl;

        ParsingBaseRequests( (*json_document_).GetRoot().AsMap().at("base_requests"s));

//        if ((*json_document_).GetRoot().IsMap())
//            std::cerr<<"KO2"s<<std::endl;

    }

    void WriteJSON (std::ostream& output) {
        using std::literals::string_literals::operator""s;

//        if (!(*json_document_).GetRoot().IsMap())
//            throw std::runtime_error ("JSON not load"s);
        if (json_document_)
            json::Print(ParsingStatRequests((*json_document_).GetRoot().AsMap().at("stat_requests"s)), output);

    }

};

}
