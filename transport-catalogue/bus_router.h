#pragma once

#include <optional>
#include <list>
#include <vector>
#include <unordered_map>
#include <string>

#include "domain.h"
#include "graph.h"
#include "router.h"

namespace router {

struct EdgeAttribute {
    std::string_view name {};
    double time = 0.0;
    int /*size_t*/ span_count = 0;

//    EdgeAttribute() = default;
//    EdgeAttribute(double t) : time(t) {};
    bool operator< (const EdgeAttribute& rhs) const {
        return time < rhs.time;
    }

    bool operator> (const EdgeAttribute& rhs) const {
        return rhs < *this;
    }

//    bool operator== (const EdgeAttribute& rhs) const {
//        return time == rhs.time;
//    }

    EdgeAttribute operator+ (const EdgeAttribute& rhs) const {
        return EdgeAttribute{.time = time + rhs.time };
    }
};

class BusRouter {
double bus_wait_time_ = 0.0;
double bus_velocity = 1.0;

//std::vector<t_catalogue::Stop*> stop_by_number_;
std::unordered_map<t_catalogue::Stop*, size_t> number_by_stop_;
std::optional<graph::DirectedWeightedGraph<EdgeAttribute>> bus_graph_;
std::optional<graph::Router<EdgeAttribute>> router_;

//struct EdgeHasher {
//    size_t operator() (const graph::Edge<double> edge) const {
//        return static_cast<size_t>(std::hash<size_t> {} (edge.from)*137 + std::hash<size_t> {} (edge.to)*13 + std::hash<double> {} (edge.weight));
//    }
//};
//
//std::unordered_map<graph::Edge<double>, EdgeAttribute, EdgeHasher> edges_;

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

    void InitRouter(const std::vector<t_catalogue::Bus*> buses, std::list<t_catalogue::Stop*> stops) {
        for (auto stop : stops)
            number_by_stop_[stop] = number_by_stop_.size();

//        stop_by_number_ = std::move(stops);
        bus_graph_.emplace(graph::DirectedWeightedGraph<EdgeAttribute>(stops.size()));

        for (const auto bus : buses) {
            for (auto stop_ptr = bus->route.begin(); stop_ptr != bus->route.end(); ++stop_ptr){
//                auto stop = *stop_ptr;
                bus_graph_->AddEdge(graph::Edge<EdgeAttribute>{ .from = number_by_stop_.at(*stop_ptr),
                                                                .to = number_by_stop_.at(*stop_ptr),
                                                                .weight = { .name = (*stop_ptr)->name,
                                                                            .time = bus_wait_time_,
                                                                            .span_count = 0
                                                                            }
                                                                });
                size_t rout_distance = 0;
//                for (size_t rout_distanse = 0, offset_ptr = 1; stop_ptr+offset_ptr != bus->route.end(); ++offset_ptr ) {
                for (auto stop_to_ptr = std::next(stop_ptr); stop_to_ptr != bus->route.end(); ++stop_to_ptr ) {
                    rout_distance+= bus->stop_to_stop_length[std::distance(bus->route.begin(),stop_to_ptr) -1 ];
                    bus_graph_->AddEdge(graph::Edge<EdgeAttribute>{ .from = number_by_stop_.at(*stop_ptr),
                                                                    .to = number_by_stop_.at(*stop_to_ptr),
                                                                    .weight = { .name = bus->name,
                                                                                .time = rout_distance / bus_velocity,
                                                                                .span_count = std::distance(stop_ptr, stop_to_ptr)
                                                                                }
                                                                    });
                }
            }
        }

        router_.emplace(graph::Router<EdgeAttribute>(*bus_graph_));
    }

    std::pair<double,std::vector<EdgeAttribute>> BuilRoute(t_catalogue::Stop* from, t_catalogue::Stop* to) const{
        auto route_info = router_->BuildRoute(number_by_stop_.at(from),number_by_stop_.at(to));
        if (!route_info)
            throw std::out_of_range("Stop [ " + from->name + " ] and [ " + to->name + " ] are not connected");
        std::vector<EdgeAttribute> ans;
        ans.reserve(route_info->edges.size());
        for (auto edge_id : route_info->edges)
            ans.emplace_back(bus_graph_->GetEdge(edge_id).weight);
        return {route_info->weight.time ,ans};
    }
//
//struct RouteInfo {
//        Weight weight;
//        std::vector<EdgeId> edges;
//    };
//
//    std::optional<RouteInfo> BuildRoute(VertexId from, VertexId to) const;

};


}
