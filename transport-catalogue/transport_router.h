#pragma once

#include "router.h"
#include "transport_catalogue.h"
#include <memory>

namespace transport_router {

    class TransportRouter {
    public:
        explicit TransportRouter(transport_catalogue::RoutingSettings& settings) : settings_(settings){}

        void BuildTransportRouter(const transport_catalogue::TransportCatalogue& catalogue) {

        }

    private:
        transport_catalogue::RoutingSettings& settings_;
        std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
        std::unique_ptr<graph::Router<double>> router_;
        //std::unordered_map<transport_catalogue::stop::Stop*, StopPairVertexId> stop_as_pair_number_;
        //std::unordered_map<graph::EdgeId, EdgeInfo> edge_id_to_type_;
    };

}