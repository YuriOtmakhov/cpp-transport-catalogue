#include "transport_catalogue.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <utility>

#include "geo.h"


size_t TransportCatalogue::DistanceHasher::operator() (const std::pair<std::string_view, std::string_view> Stops) const {
    return static_cast<size_t>(std::hash<std::string_view> {} (Stops.first)*137 + std::hash<std::string_view> {} (Stops.second));
}

TransportCatalogue::~TransportCatalogue () {
    for (const auto[name, bus]: name_to_bus_)
        delete bus;
    for (const auto[name, stop]: name_to_stops_)
        delete stop;
}

void TransportCatalogue::AddStop(std::string_view str, const double latitude, const double longitude) {
    Stop* stop_ptn = new Stop { static_cast<std::string>(str),{latitude,longitude}};
    name_to_stops_[stop_ptn->name] = stop_ptn;
    stop_to_bus_[stop_ptn->name] = {};
};

void TransportCatalogue::AddBus(std::string_view str, const std::vector<std::string_view> stop_array) {
    Bus* bus_ptn = new Bus;
    bus_ptn->name = static_cast<std::string>(str);
    for (auto stop : stop_array) {
        bus_ptn->route.push_back(FindStop(stop));
        stop_to_bus_[stop].insert(bus_ptn);
    }
    name_to_bus_[bus_ptn->name] = bus_ptn;
};

void TransportCatalogue::AddDistance(std::string_view stop_a, const std::list<std::pair<std::string_view, size_t>> distance_array) {
    std::string_view view_stop_a = FindStop(stop_a)->name;

    for (const auto& [stop_b, distance]: distance_array)
        stop_to_stop_distance_[ std::make_pair( view_stop_a, static_cast<std::string_view>(FindStop(stop_b)->name))] = distance;
}

TransportCatalogue::Stop* TransportCatalogue::FindStop(const std::string_view str) const {
    if (!name_to_stops_.count(str))
        throw std::out_of_range("Stop not found");
    return name_to_stops_.at(str);
}

TransportCatalogue::Bus* TransportCatalogue::FindBus(const std::string_view str) const {
    if (!name_to_bus_.count(str))
        throw std::out_of_range("Bus not found");
    return name_to_bus_.at(str);
}

BusDate TransportCatalogue::GetBusInfo (std::string_view str) const {
    BusDate ans;
    Bus* bus_ = FindBus(str);
    ans.length = 0.0;
    ans.stops = bus_->route.size();
    std::set<Stop*> unique_stops;
    double geo_length = 0;
    for (size_t i =0; i+1 < ans.stops; ++i) {
        geo_length += ComputeDistance(bus_->route[i]->coordinates, bus_->route[i+1]->coordinates);
        ans.length += GetDistance(bus_->route[i]->name, bus_->route[i+1]->name);
        unique_stops.insert(bus_->route[i]);
    }
    unique_stops.insert(bus_->route[ans.stops - 1]);
    ans.unique_stops = unique_stops.size();
    ans.curvature = static_cast<double>(ans.length) / geo_length;
    return ans;
}

std::vector<std::string_view> TransportCatalogue::GetStopInfo (std::string_view str) const {
    if (!stop_to_bus_.count(str))
        throw std::out_of_range("Stop not found");
    std::vector<std::string_view> ans;
    for (Bus* bus: stop_to_bus_.at(str))
        ans.push_back( bus->name );
    std::sort(ans.begin(), ans.end());
    return ans;
}

size_t TransportCatalogue::GetDistance (const std::string_view stop_a, const std::string_view stop_b) const{
    if (stop_to_stop_distance_.count({stop_a,stop_b}))
        return stop_to_stop_distance_.at({stop_a,stop_b});
    else if (stop_to_stop_distance_.count({stop_b,stop_a}))
        return stop_to_stop_distance_.at({stop_b,stop_a});

    throw std::out_of_range("Stop not found");
}
