#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

namespace geo {

namespace constant {

const int R_EARTH = 6371000;
const double DR = M_PI / 180.0;

}

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const;
    bool operator!=(const Coordinates& other) const;
};

inline double ComputeDistance(Coordinates from, Coordinates to) {
    if (from == to) {
        return 0;
    }
    using constant::R_EARTH, constant::DR;
    return std::acos(sin(from.lat * DR) * std::sin(to.lat * DR)
                + std::cos(from.lat * DR) * std::cos(to.lat * DR) * std::cos(std::abs(from.lng - to.lng) * DR))
        *  R_EARTH;
}

}
