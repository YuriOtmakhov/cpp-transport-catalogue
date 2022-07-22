#include "transport_catalogue.h"

#include <utility>
#include <algorithm>
#include <stdexcept>

using namespace t_catalogue;

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
//    Stop* stop_ptn = new Stop { static_cast<std::string>(stop_name),{latitude,longitude},{}};
    stops_.push_back(new Stop { static_cast<std::string>(stop_name),{latitude,longitude},{}});
    name_to_stops_[stops_.back()->name] = stops_.back();
}

void TransportCatalogue::AddBus(std::string_view bus_name, const std::vector<std::string_view> stop_array, bool is_round) {
    buses_.push_back(new Bus);
    buses_.back()->name = static_cast<std::string>(bus_name);

    std::set<std::string_view> unique_stops;
    double geo_length = 0.0;

    buses_.back()->route.push_back(FindStop(stop_array.front()));
    FindStop(stop_array.front())->buses.insert(buses_.back());
    unique_stops.insert(stop_array.front());

    for (auto It = std::next(stop_array.begin()); It != stop_array.end(); ++It) {
        Stop* old_stop = buses_.back()->route.back();

        buses_.back()->route.push_back(FindStop(*It));
        FindStop(*It)->buses.insert(buses_.back());
        unique_stops.insert(*It);

        geo_length += geo::ComputeDistance(old_stop->coordinates, buses_.back()->route.back()->coordinates);
        buses_.back()->length += GetDistance(old_stop, buses_.back()->route.back());

    }

    buses_.back()->unique_stops = unique_stops.size();
    buses_.back()->curvature = static_cast<double>(buses_.back()->length) / geo_length;
    buses_.back()->is_round = is_round;
    name_to_bus_[buses_.back()->name] = buses_.back();

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
        throw std::out_of_range("Bus not found: " + static_cast<std::string>(str));
    return name_to_bus_.at(str);
}

size_t TransportCatalogue::GetDistance (Stop* stop_a, Stop* stop_b) const{
    if (stop_to_stop_distance_.count({stop_a,stop_b}))
        return stop_to_stop_distance_.at({stop_a,stop_b});
    return stop_to_stop_distance_.at({stop_b,stop_a});
}

const std::list<Bus*> TransportCatalogue::GetAllBus () const {
//    std::list<Bus*> bus_list;
//    for (const auto& [key, bus] : name_to_bus_)
//        bus_list.push_back(bus);
//    return std::move(bus_list);
    return buses_;
}

const std::list<Stop*> TransportCatalogue::GetAllStops () const {
//    std::list<Stop*> stop_list;
//    for (const auto& [key, stop] : name_to_stops_)
//        stop_list.push_back(stop);
//    return std::move(bus_list);
    return stops_;
}
