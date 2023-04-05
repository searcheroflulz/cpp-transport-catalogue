#include "transport_router.h"

namespace transport_router {

    void TransportRouter::BuildTransportRouter(transport_catalogue::TransportCatalogue& catalogue) {
        graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(catalogue.GetAllStops().size() * 2);
        GetStops(catalogue.GetAllStops());
        LoadWaitEdges();
        LoadBusEdges(catalogue);
        router_ = std::make_unique<graph::Router<double>>(*graph_);
    }

    void TransportRouter::GetStops(const std::unordered_map<std::string_view, transport_catalogue::stop::Stop *>& stops) {
        size_t count = 0;
        for (auto stop: stops) {
            graph::VertexId first_id = count++;
            graph::VertexId second_id = count++;
            stop_as_pair_number_[stop.second] = StopPairVertexId{first_id, second_id};
        }
    }

    void TransportRouter::LoadWaitEdges() {
        for (const auto& [stop, pair_vertex_id]: stop_as_pair_number_) {
            graph::EdgeId edge_id = graph_->AddEdge({pair_vertex_id.bus_wait_begin, pair_vertex_id.bus_wait_end,static_cast<double>(settings_.bus_wait_time_)});
            edgeid_to_edgeinfo_[edge_id] = WaitEdgeInfo{stop->name, static_cast<double>(settings_.bus_wait_time_)};
        }
    }

    void TransportRouter::LoadBusEdges(transport_catalogue::TransportCatalogue catalogue) {
        auto& buses = catalogue.GetAllBuses();
        for (const auto& [name, bus]: buses) {
            if (bus->circle) {
                ProcessRoute(bus->route.begin(), bus->route.end(), catalogue, name);
            } else {
                ProcessRoute(bus->route.begin(), bus->route.end(), catalogue, name);
                ProcessRoute(bus->route.rbegin(), bus->route.rend(), catalogue, name);
            }
        }
    }

    graph::Edge<double> TransportRouter::BuildBusEdge(transport_catalogue::stop::Stop *from, transport_catalogue::stop::Stop *to, const double distance) const {
        return {stop_as_pair_number_.at(from).bus_wait_end, stop_as_pair_number_.at(to).bus_wait_begin,
                (distance / 1000.0 / (settings_.bus_velocity_) * 60)};
    }

    std::optional<StopPairVertexId> TransportRouter::GetPairVertexId(transport_catalogue::stop::Stop *stop) const {
        if (stop_as_pair_number_.count(stop)) {
            return {stop_as_pair_number_.at(stop)};
        }
        return std::nullopt;
    }

    const std::variant<WaitEdgeInfo, BusEdgeInfo>& TransportRouter::GetEdgeInfo(graph::EdgeId id) const {
        return edgeid_to_edgeinfo_.at(id);
    }

    std::optional<RouteInfo> TransportRouter::GetRouteInfo(graph::VertexId from, graph::VertexId to) const {
        std::optional<typename graph::Router<double>::RouteInfo> route_info = router_->BuildRoute(from, to);
        if (route_info) {
            RouteInfo result;
            result.total_time = route_info->weight;
            for (const auto edge: route_info->edges) {
                result.edges.emplace_back(GetEdgeInfo(edge));
            }
            return result;
        }
        return std::nullopt;
    }

    transport_catalogue::RoutingSettings& TransportRouter::GetRoutingSettings(){
        return settings_;
    }

    const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
        return *graph_;
    }

    const graph::Router<double>& TransportRouter::GetRouter() const {
        return *router_;
    }

    const std::unordered_map<transport_catalogue::stop::Stop*, StopPairVertexId>& TransportRouter::GetStopAsPairNumber() const {
        return stop_as_pair_number_;
    }

    const std::unordered_map<graph::EdgeId, std::variant<WaitEdgeInfo, BusEdgeInfo>> TransportRouter::GetEdgeidToType() const {
        return edgeid_to_edgeinfo_;
    }

    void TransportRouter::SetGraph(std::vector<graph::Edge<double>>&& edges, std::vector<graph::DirectedWeightedGraph<double>::IncidenceList>&& incidence_lists) {
        graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(move(edges), move(incidence_lists));
    }

    void TransportRouter::SetStopAsPairNumber(std::unordered_map<transport_catalogue::stop::Stop*, StopPairVertexId>&& stop_as_pair_number) {
        stop_as_pair_number_ = std::move(stop_as_pair_number);
    }

    void TransportRouter::SetEdgeidToType(std::unordered_map<graph::EdgeId, EdgeInfo>&& edge_id_to_type) {
        edgeid_to_edgeinfo_ = std::move(edge_id_to_type);
    }

    void TransportRouter::SetRouter(graph::Router<double>::RoutesInternalData&& routes_internal_data) {
        router_ = std::make_unique<graph::Router<double>>(*graph_, std::move(routes_internal_data));
    }
}