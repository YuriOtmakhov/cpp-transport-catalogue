#pragma once

#include <iostream>
#include <optional>
#include <vector>
#include <string>

#include "request_handler.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"
#include "json_builder.h"
#include "bus_router.h"

namespace json_reader {

class JsonReader {

    t_catalogue::TransportCatalogue* const catalogue_;
    request_handler::RequestHandler* const handler_;
    renderer::MapRenderer* const render_;
    router::BusRouter* const router_;

    std::optional<json::Document> json_document_;

    std::vector<std::string_view> ParsQueryRoute (const json::Array stops, bool is_round) const;

    void ParsingBaseRequests (const json::Node& document);

    json::Node StopRequests(const json::Node request) const;

    json::Node BusRequests(const json::Node request) const;

    json::Node MapRequests(const json::Node request) const;

    json::Node RouteRequests(const json::Node request) const;

    const json::Document ParsingStatRequests(const json::Node& document) const;

    auto ParsingColor (const json::Node& color) const;

    void ParsingRenderSettings (const json::Node& settings);

    void ParsingRoutingSettings(const json::Node& settings);

public:
    explicit JsonReader(request_handler::RequestHandler* const handler, renderer::MapRenderer* const render, router::BusRouter* const router) :
        catalogue_(handler->GetTransportCatalogue ()),
        handler_(handler),
        render_(render),
        router_(router) {
    };

    void ReadJSON (std::istream& input);

    void WriteJSON (std::ostream& output) const;

};

}
