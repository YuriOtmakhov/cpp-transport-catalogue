#pragma once

#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include "domain.h"

namespace t_catalogue {

class TransportCatalogue {

struct DistanceHasher {
    size_t operator() (const std::pair<Stop*, Stop*> Stops) const;
};

std::unordered_map<std::string_view, Stop*> name_to_stops_;
std::unordered_map<std::string_view, Bus*> name_to_bus_;
std::unordered_map<std::pair<Stop*, Stop*>, size_t, DistanceHasher> stop_to_stop_distance_;
std::list<Stop*> stops_;
std::list<Bus*> buses_;

size_t GetDistance (Stop* stop_a, Stop* stop_b) const;

public:

    ~TransportCatalogue ();

    void AddStop(std::string_view str, const double latitude, const double longitude);

    void AddBus(std::string_view str, const std::vector<std::string_view> stop_array);

    void AddDistance(std::string_view stop_a, const std::list<std::pair<std::string_view, size_t>> distance_array);

    const std::list<Bus*> GetAllBus () const;

    const std::list<Stop*> GetAllStops () const;

    Stop* FindStop(const std::string_view str) const;

    Bus* FindBus(const std::string_view str) const;

};
}
