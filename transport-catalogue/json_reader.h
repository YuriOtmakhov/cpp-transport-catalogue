#pragma once

#include <iostream>
#include <optional>
#include <vector>
#include <string>

#include "request_handler.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"

namespace json_reader {

class JsonReader {

t_catalogue::TransportCatalogue* const catalogue_;
request_handler::RequestHandler* const handler_;
renderer::MapRenderer* const render_;
std::optional<json::Document> json_document_;

std::vector<std::string_view> ParsQueryRoute (const json::Array stops, bool is_round) const;

void ParsingBaseRequests (const json::Node& document);

json::Node StopRequests(const json::Node request) const;

json::Node BusRequests(const json::Node request) const;

const json::Document ParsingStatRequests(const json::Node& document) const;

void ParsingColorUnderlayerSettings (const json::Node& color);

void ParsingColorPaletteSettings (const json::Node& color);

void ParsingRenderSettings (const json::Node& settings);

public:
    explicit JsonReader(request_handler::RequestHandler* const handler, renderer::MapRenderer* const render) :
        catalogue_(handler->GetTransportCatalogue ()),
        handler_(handler),
        render_(render) {
    };

    void ReadJSON (std::istream& input);

    void WriteJSON (std::ostream& output) const;

};

}
