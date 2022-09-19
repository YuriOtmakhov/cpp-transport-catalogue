#pragma once

#include <optional>
#include <list>
#include <vector>
#include <unordered_map>

#include "domain.h"
#include "graph.h"
#include "router.h"

namespace router {

struct EdgeAttribute {
    std::string name;
    double time;
    size_t span_count;
};

class BusRouter {
int bus_wait_time_ = 0;
double bus_velocity = 1.0;

std::vector<t_catalogue::Stop*> stop_by_number_;
std::unordered_map<t_catalogue::Stop*, size_t> number_by_stop_;
std::optional<graph::DirectedWeightedGraph<double>> bus_graph_;
std::optional<graph::Router<double>> router_;

struct EdgeHasher {
    size_t operator() (const graph::Edge<double> edge) const {
        return static_cast<size_t>(std::hash<size_t> {} (edge.from)*137 + std::hash<size_t> {} (edge.to)*13 + std::hash<double> {} (edge.weight));
    }
};

std::unordered_map<graph::Edge<double>, EdgeAttribute, EdgeHasher> edges_;

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

    void InitRouter(const std::vector<t_catalogue::Bus*> buses, std::vector<t_catalogue::Stop*> stops) {
        stop_by_number_ = std::move(stops);
        bus_graph_.emplace(graph::DirectedWeightedGraph<double>(stop_by_number_.size()));
        for (auto stop : stop_by_number_)
            number_by_stop_[stop] = number_by_stop_.size();

        for (const auto bus : buses) {
            for (auto stop_ptr = bus->route.begin(); stop_ptr != bus->route.end(); ++stop_ptr){
                graph::Edge<double> edge(from = number_by_stop_[stop_ptr]; to = number_by_stop_[stop_ptr]; weight =bus_wait_time_;);
                edges_[]
            }


        }

//        template <typename Weight>
//        struct Edge {
//        VertexId from;
//        VertexId to;
//        Weight weight;
//        };
        router_.emplace(graph::Router<double>(*bus_graph_));
    }

};


}
