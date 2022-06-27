#pragma once

#include <list>
#include <string>
#include <vector>

#include "transport_catalogue.h"
//#include "map_renderer.h"

namespace request_handler {

class RequestHandler {

transport_catalogue::TransportCatalogue* const catalogue_;
//renderer::MapRenderer* const renderer_;

public:
    explicit RequestHandler () : catalogue_(new transport_catalogue::TransportCatalogue) {
    };

    explicit RequestHandler (transport_catalogue::TransportCatalogue* const catalogue) : catalogue_(catalogue) {
    };

    transport_catalogue::TransportCatalogue* GetTransportCatalogue () const;

    transport_catalogue::BusDate GetBusInfo (std::string_view name) const;

    std::vector<transport_catalogue::Bus*> GetBusesByStop (std::string_view stop_name) const;

//    svg::Document RenderMap() const;

};

}


