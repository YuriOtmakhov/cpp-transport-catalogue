#pragma once

#include <optional>
#include <list>

#include "graph.h"
#include "router.h"

namespace router {

class BusRouter {
int bus_wait_time_ = 0;
double bus_velocity = 1.0;
std::optional<graph::Router<double>> router_;

public:
    BusRouter& SetWaitTime(int time) {
        bus_wait_time_ = time;
        return *this;
    }

    BusRouter& SetVelocity(double velosity) {
        bus_velocity = velosity;
        return *this;
    }

    bool IsInit() const {
        return router_.has_value();
    }

    operator bool () const{
        return IsInit();
    }

    void InitRouter(const std::vector<t_catalogue::Bus*>buses, const std::list<t_catalogue::Stop*> stops) {
        graph::DirectedWeightedGraph<double> grap(stops.size());
//        template <typename Weight>
//        struct Edge {
//        VertexId from;
//        VertexId to;
//        Weight weight;
//        };

    }

};


}
