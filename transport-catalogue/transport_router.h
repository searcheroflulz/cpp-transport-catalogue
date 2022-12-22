#pragma once

#include "router.h"
#include "transport_catalogue.h"
#include <memory>
#include "domain.h"

namespace transport_router {

    class TransportRouter {
    public:
        explicit TransportRouter(transport_catalogue::RoutingSettings& settings) : settings_(settings){}

        void BuildTransportRouter(transport_catalogue::TransportCatalogue& catalogue) {
            graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(catalogue.GetAllStops().size() * 2);
            FillStopIdDictionaries(catalogue.GetAllStops());
            AddWaitEdges();
            AddBusEdges(catalogue);
            router_ = std::make_unique<graph::Router<double>>(*graph_);
            //router_->Build();
        }

        void FillStopIdDictionaries(const std::unordered_map<std::string_view, transport_catalogue::stop::Stop*>& stops) {
            size_t index = 0;
            for (auto stop : stops) {
                graph::VertexId first_id = index++;
                graph::VertexId second_id = index++;
                stop_as_pair_number_[stop.second] = StopPairVertexId{first_id, second_id};
            }
        }

        void AddWaitEdges() {
            for (const auto& [stop, pair_vertex_id] : stop_as_pair_number_) {
                graph::EdgeId edge_id = graph_->AddEdge({pair_vertex_id.bus_wait_begin, pair_vertex_id.bus_wait_end,
                                                         static_cast<double>(settings_.bus_wait_time_)});
                edge_id_to_type_[edge_id] = WaitEdgeInfo{stop->name, static_cast<double>(settings_.bus_wait_time_)};
            }
        }

        void AddBusEdges(transport_catalogue::TransportCatalogue catalogue) {
            auto& buses = catalogue.GetAllBuses();
            for (const auto& [bus_name, bus] : buses) {
                if (bus->circle) {
                    ParseBusRouteOnEdges(bus->route.begin(), bus->route.end(), catalogue, bus_name);
                } else {
                    ParseBusRouteOnEdges(bus->route.begin(), bus->route.end(), catalogue, bus_name);
                    ParseBusRouteOnEdges(bus->route.rbegin(), bus->route.rend(), catalogue, bus_name);
                }
            }
        }

        template <typename InputIt>
        void ParseBusRouteOnEdges(InputIt range_begin, InputIt range_end, transport_catalogue::TransportCatalogue transport_catalogue,
                                  std::string_view bus) {
            for (auto stop_from = range_begin; stop_from != range_end; ++stop_from) {
                size_t distance = 0;
                size_t span_count = 0;
                for (auto stop_to = next(stop_from); stop_to != range_end; ++stop_to) {
                    auto befor_stop_to = prev(stop_to);
                    distance += transport_catalogue.GetDistanceBetween(*befor_stop_to, *stop_to);
                    ++span_count;
                    graph::EdgeId edge_id = graph_->AddEdge({MakeBusEdge(*stop_from, *stop_to, distance)});
                    edge_id_to_type_[edge_id] = BusEdgeInfo{bus, span_count, graph_->GetEdge(edge_id).weight};
                }
            }
        }

        graph::Edge<double> MakeBusEdge(transport_catalogue::stop::Stop* from, transport_catalogue::stop::Stop* to, const double distance) const {
            return {stop_as_pair_number_.at(from).bus_wait_end, stop_as_pair_number_.at(to).bus_wait_begin, (distance / 1000.0 / (settings_.bus_velocity_) * 60)};
        }

        std::optional<StopPairVertexId> GetPairVertexId(transport_catalogue::stop::Stop* stop) const {
            if (stop_as_pair_number_.count(stop)) {
                return {stop_as_pair_number_.at(stop)};
            }
            return std::nullopt;
        }

        const std::variant<WaitEdgeInfo, BusEdgeInfo>& GetEdgeInfo(graph::EdgeId id) const {
            return edge_id_to_type_.at(id);
        }

        std::optional<RouteInfo> GetRouteInfo(graph::VertexId from, graph::VertexId to) const {
            std::optional<typename graph::Router<double>::RouteInfo> route_info = router_->BuildRoute(from, to);
            if (route_info) {
                RouteInfo result;
                result.total_time = route_info->weight;
                for (const auto edge : route_info->edges) {
                    result.edges.emplace_back(GetEdgeInfo(edge));
                }
                return result;
            }
            return std::nullopt;
        }


    private:
        transport_catalogue::RoutingSettings& settings_;
        std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
        std::unique_ptr<graph::Router<double>> router_;
        std::unordered_map<transport_catalogue::stop::Stop*, StopPairVertexId> stop_as_pair_number_;
        std::unordered_map<graph::EdgeId, std::variant<WaitEdgeInfo, BusEdgeInfo>> edge_id_to_type_;
    };

}