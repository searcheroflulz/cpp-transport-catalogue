#pragma once

#include "router.h"
#include "transport_catalogue.h"
#include <memory>
#include "domain.h"

namespace transport_router {
    class TransportRouter {
    public:
        explicit TransportRouter(transport_catalogue::RoutingSettings& settings) : settings_(settings){}

        void BuildTransportRouter(transport_catalogue::TransportCatalogue& catalogue);

        void GetStops(const std::unordered_map<std::string_view, transport_catalogue::stop::Stop*>& stops);

        void LoadWaitEdges();

        void LoadBusEdges(transport_catalogue::TransportCatalogue catalogue);

        graph::Edge<double> BuildBusEdge(transport_catalogue::stop::Stop* from, transport_catalogue::stop::Stop* to, const double distance) const;

        std::optional<StopPairVertexId> GetPairVertexId(transport_catalogue::stop::Stop* stop) const;

        const std::variant<WaitEdgeInfo, BusEdgeInfo>& GetEdgeInfo(graph::EdgeId id) const;

        std::optional<RouteInfo> GetRouteInfo(graph::VertexId from, graph::VertexId to) const;

        template <typename InputIt>
        void ProcessRoute(InputIt range_begin, InputIt range_end, transport_catalogue::TransportCatalogue transport_catalogue, std::string_view bus);

        private:
            transport_catalogue::RoutingSettings& settings_;
            std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
            std::unique_ptr<graph::Router<double>> router_;
            std::unordered_map<transport_catalogue::stop::Stop*, StopPairVertexId> stop_as_pair_number_;
            std::unordered_map<graph::EdgeId, std::variant<WaitEdgeInfo, BusEdgeInfo>> edgeid_to_edgeinfo_;
        };

        template <typename InputIt>
        void TransportRouter::ProcessRoute(InputIt range_begin, InputIt range_end, transport_catalogue::TransportCatalogue transport_catalogue, std::string_view bus) {
            for (auto stop_from = range_begin; stop_from != range_end; ++stop_from) {
                size_t distance = 0;
                size_t span_count = 0;
                for (auto stop_to = next(stop_from); stop_to != range_end; ++stop_to) {
                    auto before_stop_to = prev(stop_to);
                    distance += transport_catalogue.GetDistanceBetween(*before_stop_to, *stop_to);
                    ++span_count;
                    graph::EdgeId edge_id = graph_->AddEdge({BuildBusEdge(*stop_from, *stop_to, distance)});
                    edgeid_to_edgeinfo_[edge_id] = BusEdgeInfo{bus, span_count, graph_->GetEdge(edge_id).weight};
                }
            }
        }
    }