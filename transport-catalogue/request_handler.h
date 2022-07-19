#pragma once

#include <list>
#include <string>
#include <vector>

#include "transport_catalogue.h"
//#include "map_renderer.h"

namespace request_handler {

class RequestHandler {

t_catalogue::TransportCatalogue* const catalogue_;
//renderer::MapRenderer* const renderer_;

public:
    explicit RequestHandler () : catalogue_(new t_catalogue::TransportCatalogue) {
    };

    explicit RequestHandler (t_catalogue::TransportCatalogue* const catalogue) : catalogue_(catalogue) {
    };

    t_catalogue::TransportCatalogue* GetTransportCatalogue () const;

    t_catalogue::BusDate GetBusInfo (std::string_view name) const;

    const std::vector<t_catalogue::Bus*> GetBusesByStop (std::string_view stop_name) const;

    const std::vector<t_catalogue::Bus*> GetAllRound () const;

//    svg::Document RenderMap() const;

};

}


