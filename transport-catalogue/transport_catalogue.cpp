#include "transport_catalogue.h"


#include <utility>
#include <algorithm>
#include <stdexcept>

using namespace transport_catalogue;

size_t TransportCatalogue::DistanceHasher::operator() (const std::pair<Stop*, Stop*> Stops) const {
    return static_cast<size_t>(std::hash<Stop*> {} (Stops.first)*137 + std::hash<Stop*> {} (Stops.second));
}

TransportCatalogue::~TransportCatalogue () {
    for (const auto[name, bus]: name_to_bus_)
        delete bus;
    for (const auto[name, stop]: name_to_stops_)
        delete stop;
}

void TransportCatalogue::AddStop(std::string_view stop_name, const double latitude, const double longitude) {
    Stop* stop_ptn = new Stop { static_cast<std::string>(stop_name),{latitude,longitude},{}};
    name_to_stops_[stop_ptn->name] = stop_ptn;
//    stop_to_bus_[stop_ptn->name] = {};
}

void TransportCatalogue::AddBus(std::string_view bus_name, const std::vector<std::string_view> stop_array) {
    Bus* bus_ptn = new Bus;
    bus_ptn->name = static_cast<std::string>(bus_name);

    std::set<std::string_view> unique_stops;
    double geo_length = 0.0;

    bus_ptn->route.push_back(FindStop(stop_array.front()));
    FindStop(stop_array.front())->buses.insert(bus_ptn);
    unique_stops.insert(stop_array.front());

    for (auto It = std::next(stop_array.begin()); It != stop_array.end(); ++It) {
        Stop* old_stop = bus_ptn->route.back();

        bus_ptn->route.push_back(FindStop(*It));
        FindStop(*It)->buses.insert(bus_ptn);
        unique_stops.insert(*It);

        geo_length += detail::ComputeDistance(old_stop->coordinates, bus_ptn->route.back()->coordinates);
        bus_ptn->length += GetDistance(old_stop, bus_ptn->route.back());

    }

    bus_ptn->unique_stops = unique_stops.size();
    bus_ptn->curvature = static_cast<double>(bus_ptn->length) / geo_length;
    name_to_bus_[bus_ptn->name] = bus_ptn;

}

void TransportCatalogue::AddDistance(std::string_view stop_a, const std::list<std::pair<std::string_view, size_t>> distance_array) {
    Stop* ptr_stop_a = FindStop(stop_a);

    for (const auto& [stop_b, distance]: distance_array)
        stop_to_stop_distance_[ std::make_pair( ptr_stop_a, FindStop(stop_b))] = distance;
}

Stop* TransportCatalogue::FindStop(const std::string_view str) const {
    if (!name_to_stops_.count(str))
        throw std::out_of_range("Stop not found: " + static_cast<std::string>(str));
    return name_to_stops_.at(str);
}

Bus* TransportCatalogue::FindBus(const std::string_view str) const {
    if (!name_to_bus_.count(str))
        throw std::out_of_range("Bus not found");
    return name_to_bus_.at(str);
}

//BusDate TransportCatalogue::GetBusInfo (std::string_view name) const {
//    BusDate ans;
//    Bus* bus_ = FindBus(name);
//    ans.stops = bus_->route.size();
//
//    ans.length = bus_->length;
//
//    ans.unique_stops = bus_->unique_stops;
//    ans.curvature = bus_->curvature;
//    return ans;
//}
//
//std::vector<std::string_view> TransportCatalogue::GetStopInfo (std::string_view str) const {
//    if (!stop_to_bus_.count(str))
//        throw std::out_of_range("Stop not found");
//    std::vector<std::string_view> ans;
//    for (Bus* bus: stop_to_bus_.at(str))
//        ans.push_back( bus->name );
//    std::sort(ans.begin(), ans.end());
//    return ans;
//}

size_t TransportCatalogue::GetDistance (Stop* stop_a, Stop* stop_b) const{
    if (stop_to_stop_distance_.count({stop_a,stop_b}))
        return stop_to_stop_distance_.at({stop_a,stop_b});
//    else if (stop_to_stop_distance_.count({stop_b,stop_a}))
        return stop_to_stop_distance_.at({stop_b,stop_a});

    //throw std::out_of_range("Stop not found");
}
