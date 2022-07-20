#include "request_handler.h"

#include <algorithm>

using namespace request_handler;

t_catalogue::TransportCatalogue* RequestHandler::GetTransportCatalogue () const {
    return catalogue_;
}

t_catalogue::BusDate RequestHandler::GetBusInfo (std::string_view name) const {
    t_catalogue::BusDate ans;
    t_catalogue::Bus* bus_ = catalogue_->FindBus(name);
    ans.stops = bus_->route.size();
    ans.length = bus_->length;
    ans.unique_stops = bus_->unique_stops;
    ans.curvature = bus_->curvature;
    return ans;
}

const std::vector<t_catalogue::Bus*> RequestHandler::GetBusesByStop (std::string_view stop_name) const {
    std::vector<t_catalogue::Bus*> ans;
    const auto& buses_list = catalogue_->FindStop(stop_name)->buses;
    ans.reserve(buses_list.size());
    for (const auto& bus: buses_list)
        ans.push_back(bus);
    std::sort(ans.begin(), ans.end(), [](const auto* const rhs, const auto* const lhs) { return rhs->name < lhs->name; } );
    return std::move(ans);
}

const std::vector<t_catalogue::Bus*> RequestHandler::GetAllRound () const {
    std::vector<t_catalogue::Bus*> ans;
    const auto bus_list = catalogue_->GetAllBus();
    ans.reserve(bus_list.size());
    for (t_catalogue::Bus* bus : bus_list)
        if (!bus->route.empty())
            ans.push_back(bus);

//    const auto& buses_list = catalogue_->FindStop(stop_name)->buses;
//    ans.reserve(buses_list.size());
//    for (const auto& bus: buses_list)
//        ans.push_back(bus);
    std::sort(ans.begin(), ans.end(), [](const auto* const rhs, const auto* const lhs) { return rhs->name < lhs->name; } );
    return std::move(ans);
}

const std::vector<geo::Coordinates> RequestHandler::GetMap () const {
    std::vector<geo::Coordinates> ans;
    const auto stop_list = catalogue_->GetAllStops();
    ans.reserve(stop_list.size());
    for (t_catalogue::Stop* stop : stop_list)
        if (!stop->buses.empty())
            ans.push_back(stop->coordinates);
    return std::move(ans);
}



