#pragma once

#include <list>
#include <string>
#include <vector>
#include <algorithm>

#include "transport_catalogue.h"

namespace request_handler {

class RequestHandler {

transport_catalogue::TransportCatalogue* const catalogue_;

public:
    explicit RequestHandler () : catalogue_(new transport_catalogue::TransportCatalogue) {
    };

    explicit RequestHandler (transport_catalogue::TransportCatalogue* const catalogue) : catalogue_(catalogue) {
    };

    transport_catalogue::TransportCatalogue* GetTransportCatalogue () const {
        return catalogue_;
    }

        transport_catalogue::BusDate GetBusInfo (std::string_view name) const {
        transport_catalogue::BusDate ans;
        transport_catalogue::Bus* bus_ = catalogue_->FindBus(name);
        ans.stops = bus_->route.size();

        ans.length = bus_->length;

        ans.unique_stops = bus_->unique_stops;
        ans.curvature = bus_->curvature;
        return ans;
    }

    std::vector<transport_catalogue::Bus*> GetBusesByStop (std::string_view stop_name) const {
//    catalogue_->FindStop(stop_name);
        std::vector<transport_catalogue::Bus*> ans;
        const auto& buses_list = catalogue_->FindStop(stop_name)->buses;
        ans.reserve(buses_list.size());
        for (const auto& bus: buses_list)
            ans.push_back(bus);
        std::sort(ans.begin(), ans.end(), [](const auto* const rhs, const auto* const lhs) { return rhs->name < lhs->name; } );
        return ans;
    }

};

}


