#pragma once

#include <optional>
#include <list>
#include <vector>
#include <unordered_map>
#include <string>

#include "domain.h"
#include "graph.h"
#include "router.h"

#include "iostream"

namespace router {

struct EdgeAttribute {
    std::string_view name {};
    double time = 0.0;
    int /*size_t*/ span_count = 0;

    bool operator< (const EdgeAttribute& rhs) const;

    bool operator> (const EdgeAttribute& rhs) const;

    EdgeAttribute operator+ (const EdgeAttribute& rhs) const;
};

class BusRouter {
int bus_wait_time_ = 0;
double bus_velocity = 1.0;

std::unordered_map<t_catalogue::Stop*, size_t> number_by_stop_;
std::optional<graph::DirectedWeightedGraph<EdgeAttribute>> bus_graph_;
std::optional<graph::Router<EdgeAttribute>> router_;

public:
    BusRouter& SetWaitTime(int time);
    BusRouter& SetVelocity(double velosity);

    bool IsInit() const;
    operator bool () const;

    void InitRouter(const std::vector<t_catalogue::Bus*> buses, std::list<t_catalogue::Stop*> stops);

    std::pair<double,std::vector<EdgeAttribute>> BuilRoute(t_catalogue::Stop* from, t_catalogue::Stop* to) const;

};


}
