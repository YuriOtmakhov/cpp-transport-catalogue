#pragma once

#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include "geo.h"

namespace transport_catalogue {

struct BusDate {
    unsigned int stops = 0;
    unsigned int unique_stops = 0;
    size_t length = 0;
    double curvature = 0.0;

};

class TransportCatalogue {

struct Stop {
    std::string name;
    detail::Coordinates coordinates;
};

struct Bus {
    std::string name;
    std::vector<Stop*> route;
    unsigned int unique_stops = 0;
    size_t length = 0;
    double curvature = 0.0;
};

struct DistanceHasher {
    size_t operator() (const std::pair<std::string_view, std::string_view> Stops) const;
};

std::unordered_map<std::string_view, Stop*> name_to_stops_;
std::unordered_map<std::string_view, Bus*> name_to_bus_;
std::unordered_map<std::string_view, std::unordered_set<Bus*>> stop_to_bus_;
std::unordered_map<std::pair<std::string_view, std::string_view>, size_t, DistanceHasher> stop_to_stop_distance_;

Stop* FindStop(const std::string_view str) const;

Bus* FindBus(const std::string_view str) const;

size_t GetDistance (const std::string_view stop_a, const std::string_view stop_b) const;

public:

    ~TransportCatalogue ();

    void AddStop(std::string_view str, const double latitude, const double longitude);

    void AddBus(std::string_view str, const std::vector<std::string_view> stop_array);

    void AddDistance(std::string_view stop_a, const std::list<std::pair<std::string_view, size_t>> distance_array);

    BusDate GetBusInfo (std::string_view str) const;

    std::vector<std::string_view> GetStopInfo (std::string_view str) const;

};
}
