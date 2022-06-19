#pragma once

#include <string>
#include <vector>
#include <unordered_set>

#include "geo.h"

namespace transport_catalogue {

struct BusDate {
    unsigned int stops = 0;
    unsigned int unique_stops = 0;
    unsigned int length = 0;
    double curvature = 0.0;
};

struct Bus;

struct Stop {
    std::string name;
    detail::Coordinates coordinates;
    std::unordered_set<Bus*> buses;
};

struct Bus {
    std::string name;
    std::vector<Stop*> route;
    unsigned int unique_stops = 0;
    size_t length = 0;
    double curvature = 0.0;
};

}
