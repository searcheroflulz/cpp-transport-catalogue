#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <deque>
#include <optional>
#include <iostream>
#include <transport_router.pb.h>
#include <transport_catalogue.pb.h>
#include <svg.pb.h>
#include <unordered_map>


namespace serial_handler {
    using EdgeInfo = std::variant<WaitEdgeInfo, BusEdgeInfo>;
    class SerialHandler {
    public:
        SerialHandler(SerializationSettings&& settings) : settings_(std::move(settings)) {}

        void Serialization(std::ostream& output);

        void SerializationTransportCatalogue(transport_catalogue::TransportCatalogue& transport_catalogue);

        void SerializationMapRenderSettings(const map_renderer::MapRenderer& map_renderer);

        void SerializationTransportRouter(const transport_router::TransportRouter& transport_router);

        void Deserialize(std::istream& input);

        transport_catalogue::TransportCatalogue GetTransportCatalogue();

        map_renderer::MapRenderSettings GetMapRenderSettings();

        transport_router::TransportRouter GetTransportRouter(transport_catalogue::TransportCatalogue& transport_catalogue);

        const SerializationSettings& GetSettings() const;

        void SerializationMapRenderSettings(const map_renderer::MapRenderSettings& render_settings);

    private:
        SerializationSettings settings_;
        transport_catalogue_serialize::TransportCatalogue transport_catalogue_proto_;
        std::deque<transport_catalogue::stop::Stop> stops_;
        std::deque<transport_catalogue::bus::Bus> buses_;

    private:
        void SerializationStops(const std::unordered_map<std::string_view, transport_catalogue::stop::Stop*>& stops);

        void SerializationBuses(const std::unordered_map<std::string_view, transport_catalogue::bus::Bus*>& buses);

        void SerializationDistanceBetweenStops(const std::unordered_map<std::pair<transport_catalogue::stop::Stop*, transport_catalogue::stop::Stop*>, uint32_t, transport_catalogue::stop::hash::Hash>& distance_between_stops_);

        void DeserializeStops(transport_catalogue::TransportCatalogue& transport_catalogue);

        void DeserializeBuses(transport_catalogue::TransportCatalogue& transport_catalogue);

        void DeserializeDistanceBetweenStops(transport_catalogue::TransportCatalogue& transport_catalogue);

        map_renderer::MapRenderSettings DeserializeMapRenderSettings();

        void SerializationColor(svg_proto::Color* color_proto, const svg::Color& color);

        svg::Color DeserializeColor(const svg_proto::Color& color_proto);

        void SerializationRoutingSettings(const transport_catalogue::RoutingSettings& routing_settings);

        void SerializationGraph(const graph::DirectedWeightedGraph<double>& graph);

        void SerializationEdgesFromGraph(const std::vector<graph::Edge<double>>& edges);

        void SerializationIncidenceListsFromGraph(const std::vector<graph::DirectedWeightedGraph<double>::IncidenceList>& incidence_lists);

        void SerializationRouter(const graph::Router<double>& router);

        void SerializationStopAsPairNumber(const std::unordered_map<transport_catalogue::stop::Stop*, StopPairVertexId>& stop_as_pair_number_);

        void SerializationEdgeidToType(const std::unordered_map<graph::EdgeId, EdgeInfo>& edge_id_to_type_);

        transport_catalogue::RoutingSettings DeserializeRoutingSettings();

        std::vector<graph::Edge<double>> DeserializeEdgesFromGraph();

        std::vector<graph::DirectedWeightedGraph<double>::IncidenceList> DeserializeIncidenceListsFromGraph();

        graph::Router<double>::RoutesInternalData DeserializeRouter();

        std::unordered_map<transport_catalogue::stop::Stop*, StopPairVertexId> DeserializeStopAsPairNumber(
                transport_catalogue::TransportCatalogue transport_catalogue);

        std::unordered_map<graph::EdgeId, EdgeInfo> DeserializeEdgeidToType(
                transport_catalogue::TransportCatalogue transport_catalogue);

    };

}
