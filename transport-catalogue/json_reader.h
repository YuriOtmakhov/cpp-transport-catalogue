#pragma once

#include <iostream>
#include <optional>
#include <vector>
#include <string>

#include "request_handler.h"
#include "transport_catalogue.h"
#include "json.h"

namespace json_reader {

class JsonReader {

transport_catalogue::TransportCatalogue* const catalogue_;
request_handler::RequestHandler* const handler_;
std::optional<json::Document> json_document_;

std::vector<std::string_view> ParsQueryRoute (const json::Array stops, bool is_round) const;

void ParsingBaseRequests (const json::Node& document);

json::Node StopRequests(const json::Node request) const;

json::Node BusRequests(const json::Node request) const;

const json::Document ParsingStatRequests(const json::Node& document) const;

public:
    explicit JsonReader(request_handler::RequestHandler* const handler) : catalogue_(handler->GetTransportCatalogue ()),handler_(handler) {
    };

    void ReadJSON (std::istream& input);

    void WriteJSON (std::ostream& output) const;

};

}
