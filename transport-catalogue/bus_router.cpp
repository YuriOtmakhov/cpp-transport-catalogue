#include "bus_router.h"



using namespace router;

bool EdgeAttribute::operator< (const EdgeAttribute& rhs) const {
    return time < rhs.time;
}

bool EdgeAttribute::operator> (const EdgeAttribute& rhs) const {
    return rhs < *this;
}

EdgeAttribute EdgeAttribute::operator+ (const EdgeAttribute& rhs) const {
    return EdgeAttribute{.time = this->time + rhs.time };
}

BusRouter& BusRouter::SetWaitTime(int time) {
    bus_wait_time_ = time;
    return *this;
}

BusRouter& BusRouter::SetVelocity(double velosity) {
    bus_velocity = velosity;
    return *this;
}

bool BusRouter::IsInit() const {
    return router_.has_value();
}

BusRouter::operator bool () const{
    return IsInit();
}

void BusRouter::InitRouter(const std::vector<t_catalogue::Bus*> buses, std::list<t_catalogue::Stop*> stops) {
    for (auto stop : stops)
        number_by_stop_[stop] = number_by_stop_.size()*2;

    bus_graph_.emplace(graph::DirectedWeightedGraph<EdgeAttribute>(2*stops.size()));

    for (const auto bus : buses)
        for (auto stop_ptr = bus->route.begin(); stop_ptr != bus->route.end(); ++stop_ptr){
            bus_graph_->AddEdge(graph::Edge<EdgeAttribute>{ .from = number_by_stop_.at(*stop_ptr),
                                                            .to = number_by_stop_.at(*stop_ptr)+1,
                                                            .weight = { .name = (*stop_ptr)->name,
                                                                        .time = bus_wait_time_,
                                                                        .span_count = 0
                                                                        }
                                                            });
            size_t rout_distance = 0;
            for (auto stop_to_ptr = std::next(stop_ptr); stop_to_ptr != bus->route.end(); ++stop_to_ptr ) {
                rout_distance+= bus->stop_to_stop_length[std::distance(bus->route.begin(),stop_to_ptr) -1 ];
                bus_graph_->AddEdge(graph::Edge<EdgeAttribute>{ .from = number_by_stop_.at(*stop_ptr)+1,
                                                                .to = number_by_stop_.at(*stop_to_ptr),
                                                                .weight = { .name = bus->name,
                                                                            .time = rout_distance / (bus_velocity*100/6),
                                                                            .span_count = std::distance(stop_ptr, stop_to_ptr)
                                                                            }
                                                                });
            }
        }

    router_.emplace(graph::Router<EdgeAttribute>(*bus_graph_));
}

std::pair<double,std::vector<EdgeAttribute>> BusRouter::BuilRoute(t_catalogue::Stop* from, t_catalogue::Stop* to) const{
    auto route_info = router_->BuildRoute(number_by_stop_.at(from),number_by_stop_.at(to));
    if (!route_info)
        throw std::out_of_range("Stop [ " + from->name + " ] and [ " + to->name + " ] are not connected");
    std::vector<EdgeAttribute> ans;
    ans.reserve(route_info->edges.size());
    for (auto edge_id : route_info->edges)
        ans.emplace_back(bus_graph_->GetEdge(edge_id).weight);
    return {route_info->weight.time ,ans};
}


