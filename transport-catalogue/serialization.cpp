#include "serialization.h"
#include "svg.h"

#include <deque>
#include <graph.pb.h>
#include <variant>
#include <unordered_map>


namespace serial_handler {
    using namespace std;
    using namespace transport_catalogue;
    using namespace transport_router;
    using namespace map_renderer;
    using EdgeInfo = std::variant<WaitEdgeInfo, BusEdgeInfo>;

    const SerializationSettings& SerialHandler::GetSettings() const {
        return settings_;
    }

    void SerialHandler::Serialization(std::ostream& output) {
        transport_catalogue_proto_.SerializeToOstream(&output);
    }

    void SerialHandler::SerializationTransportCatalogue(TransportCatalogue& transport_catalogue) {
        SerializationStops(transport_catalogue.GetAllStops());
        SerializationDistanceBetweenStops(transport_catalogue.GetDistanceBetweenStops());
        SerializationBuses(transport_catalogue.GetAllBuses());
    }

    void SerialHandler::SerializationStops(const std::unordered_map<std::string_view, stop::Stop*>& stops) {
        for (auto& stop : stops) {
            size_t count = 0;
            transport_catalogue_serialize::Stop* stop_proto = transport_catalogue_proto_.add_stops();
            stop_proto->set_name(stop.second->name);
            map_renderer_proto::Coordinate* coordinate_proto = stop_proto->mutable_coordinates();
            coordinate_proto->set_lat(stop.second->coordinates.lat);
            coordinate_proto->set_lng(stop.second->coordinates.lng);
            stop_proto->set_index(count++);
        }
    }

    void SerialHandler::SerializationBuses(const std::unordered_map<std::string_view, bus::Bus*>& buses) {
        for (auto& bus : buses) {
            transport_catalogue_serialize::Bus* bus_proto = transport_catalogue_proto_.add_buses();
            bus_proto->set_name(bus.second->name);
            bus_proto->set_is_circle(bus.second->circle);
            for (size_t i = 0; i < bus.second->route.size(); ++i) {
                bus_proto->add_route(bus.second->route[i]->name);
            }
        }
    }

    void SerialHandler::SerializationDistanceBetweenStops(const std::unordered_map<std::pair<transport_catalogue::stop::Stop*, transport_catalogue::stop::Stop*>, uint32_t, transport_catalogue::stop::hash::Hash>& distance_between_stops_) {
        for (auto& [stops, distance] : distance_between_stops_) {
            transport_catalogue_serialize::DistanceBetweenStops* distance_between_stops_proto = transport_catalogue_proto_.add_distance_between_stops();
            distance_between_stops_proto->set_stop_from(stops.first->name);
            distance_between_stops_proto->set_stop_to(stops.second->name);
            distance_between_stops_proto->set_distance(distance);
        }
    }

    void SerialHandler::SerializationMapRenderSettings(const MapRenderer& map_renderer) {
        SerializationMapRenderSettings(map_renderer.GetRenderSettings());
    }

    void SerialHandler::SerializationMapRenderSettings(const MapRenderSettings& render_settings) {
        map_renderer_proto::RenderSettings* render_settings_proto = transport_catalogue_proto_.mutable_map_renderer();
        render_settings_proto->set_width(render_settings.width);
        render_settings_proto->set_height(render_settings.height);
        render_settings_proto->set_padding(render_settings.padding);
        render_settings_proto->set_line_width(render_settings.line_width);
        render_settings_proto->set_stop_radius(render_settings.stop_radius);
        render_settings_proto->set_bus_label_font_size(render_settings.bus_label_font_size);
        for (auto element : render_settings.bus_label_offset) {
            render_settings_proto->add_bus_label_offset(element);
        }
        render_settings_proto->set_stop_label_font_size(render_settings.stop_label_font_size);
        for (auto& element : render_settings.stop_label_offset) {
            render_settings_proto->add_stop_label_offset(element);
        }
        svg_proto::Color* color_proto = render_settings_proto->mutable_underlayer_color();
        SerializationColor(color_proto, render_settings.underlayer_color);
        render_settings_proto->set_underlayer_width(render_settings.underlayer_width);
        for (auto& element : render_settings.color_palette) {
            svg_proto::Color* color_proto = render_settings_proto->add_color_palette();
            SerializationColor(color_proto, element);
        }
    }

    void SerialHandler::SerializationTransportRouter(const TransportRouter& transport_router) {
        SerializationRoutingSettings(transport_router.GetRoutingSettings());
        SerializationGraph(transport_router.GetGraph());
        SerializationRouter(transport_router.GetRouter());
        SerializationStopAsPairNumber(transport_router.GetStopAsPairNumber());
        SerializationEdgeidToType(transport_router.GetEdgeidToType());
    }

    void SerialHandler::SerializationRoutingSettings(const RoutingSettings& routing_settings) {
        transport_router_proto::RoutingSettings* routing_settings_proto = transport_catalogue_proto_.mutable_transport_router()->mutable_routing_settings();
        routing_settings_proto->set_bus_velocity(routing_settings.bus_velocity_);
        routing_settings_proto->set_bus_wait_time(routing_settings.bus_wait_time_);
    }

    void SerialHandler::SerializationGraph(const graph::DirectedWeightedGraph<double>& graph) {
        graph_proto::DirectedWeightedGraph* graph_proto = transport_catalogue_proto_.mutable_transport_router()->mutable_graph();
        graph_proto->set_vertex_count(graph.GetEdgeCount());
        SerializationEdgesFromGraph(graph.GetEdges());
        SerializationIncidenceListsFromGraph(graph.GetIncidenceList());
    }

    void SerialHandler::SerializationEdgesFromGraph(const std::vector<graph::Edge<double>>& edges) {
    graph_proto::DirectedWeightedGraph* graph_proto = transport_catalogue_proto_.mutable_transport_router()->mutable_graph();
    for (auto& edge : edges) {
    graph_proto::Edge* edge_proto = graph_proto->add_edges();
    edge_proto->set_from_edge(edge.from);
    edge_proto->set_to_edge(edge.to);
    edge_proto->set_weight(edge.weight);
}
}

void SerialHandler::SerializationIncidenceListsFromGraph(const std::vector<graph::DirectedWeightedGraph<double>::IncidenceList>& incidence_lists) {
    graph_proto::DirectedWeightedGraph* graph_proto = transport_catalogue_proto_.mutable_transport_router()->mutable_graph();
    for (auto& incidence_list : incidence_lists) {
        graph_proto::IncidenceList* incidence_list_proto = graph_proto->mutable_incidence_lists()->Add();
        for (auto edge_id: incidence_list) {
            incidence_list_proto->add_edge_id(edge_id);
        }
    }
}


void SerialHandler::SerializationRouter(const graph::Router<double>& router) {
    graph_proto::Router* router_proto = transport_catalogue_proto_.mutable_transport_router()->mutable_router();
    const typename graph::Router<double>::RoutesInternalData& routes_internal_data = router.GetRoutesInternalData();
    for (const auto& route : routes_internal_data) {
        graph_proto::RoutesInternalData* routes_internal_data_proto = router_proto->add_routes_internal_data();
        for (const auto& data : route) {
            graph_proto::RouteInternalData* route_internal_data_proto = routes_internal_data_proto->add_route_internal_data();
            if (data) {
                route_internal_data_proto->set_has_data(true);
                route_internal_data_proto->set_weight(data->weight);
                if (data->prev_edge) {
                    route_internal_data_proto->set_has_prev_edge(true);
                    route_internal_data_proto->set_prev_edge(data->prev_edge.value());
                } else {
                    route_internal_data_proto->set_has_prev_edge(false);
                }
            } else {
                route_internal_data_proto->set_has_data(false);
                route_internal_data_proto->set_has_prev_edge(false);
            }
        }
    }
}

void SerialHandler::SerializationStopAsPairNumber(const std::unordered_map<transport_catalogue::stop::Stop*, StopPairVertexId>& stop_as_pair_number) {
    auto& stop_as_pair_number_proto = *transport_catalogue_proto_.mutable_transport_router()->mutable_stop_id_to_pair_vertex_id();
    for (const auto& [stop, vertex] : stop_as_pair_number) {
        transport_router_proto::StopPairVertexId  stop_pair_vertex_id;
        stop_pair_vertex_id.set_bus_wait_begin(vertex.bus_wait_begin);
        stop_pair_vertex_id.set_bus_wait_end(vertex.bus_wait_end);
        stop_as_pair_number_proto[string(stop->name)] = move(stop_pair_vertex_id);
    }
}

void SerialHandler::SerializationEdgeidToType(const std::unordered_map<graph::EdgeId, EdgeInfo>& edge_id_to_type) {
    auto& edge_id_to_type_proto = *transport_catalogue_proto_.mutable_transport_router()->mutable_edge_id_to_type();
    for (const auto& [id, info] : edge_id_to_type) {
        transport_router_proto::EdgeInfo edge_info_proto;
        if (info.index() == 0) {
            transport_router_proto::WaitEdgeInfo wait_edge_proto;
            const WaitEdgeInfo& wait_edge = get<WaitEdgeInfo>(info);
            wait_edge_proto.set_minutes(wait_edge.time);
            wait_edge_proto.set_stop_name(string(wait_edge.stop_name));
            *edge_info_proto.mutable_wait_edge_info() = move(wait_edge_proto);
        } else {
            transport_router_proto::BusEdgeInfo bus_edge_proto;
            const BusEdgeInfo& bus_edge = get<BusEdgeInfo>(info);
            bus_edge_proto.set_bus_name(string(bus_edge.bus_name));
            bus_edge_proto.set_span_count(bus_edge.span_count);
            bus_edge_proto.set_time(bus_edge.time);
            *edge_info_proto.mutable_bus_edge_info() = move(bus_edge_proto);
        }
        edge_id_to_type_proto[id] = move(edge_info_proto);
    }
}

void SerialHandler::SerializationColor(svg_proto::Color* color_proto, const svg::Color& color) {
    if (holds_alternative<string>(color)) {
        string get_color = std::get<string>(color);
        color_proto->set_is_string(true);
        color_proto->set_color(get_color);
    } else if (holds_alternative<svg::Rgb>(color)) {
        svg::Rgb get_color = get<svg::Rgb>(color);
        color_proto->set_is_rgb(true);
        color_proto->set_red(get_color.red);
        color_proto->set_green(get_color.green);
        color_proto->set_blue(get_color.blue);
    } else {
        svg::Rgba get_color = get<svg::Rgba>(color);
        color_proto->set_is_rgba(true);
        color_proto->set_red(get_color.red);
        color_proto->set_green(get_color.green);
        color_proto->set_blue(get_color.blue);
        color_proto->set_opacity(get_color.opacity);
    }
}

void SerialHandler::Deserialize(std::istream& input) {
    transport_catalogue_proto_.Clear();
    transport_catalogue_proto_.ParseFromIstream(&input);
}

TransportCatalogue SerialHandler::GetTransportCatalogue() {
    TransportCatalogue transport_catalogue;
    DeserializeStops(transport_catalogue);
    DeserializeDistanceBetweenStops(transport_catalogue);
    DeserializeBuses(transport_catalogue);
    return transport_catalogue;
}

MapRenderSettings SerialHandler::GetMapRenderSettings() {
    return DeserializeMapRenderSettings();
}

TransportRouter SerialHandler::GetTransportRouter(TransportCatalogue& transport_catalogue) {
        auto settings = DeserializeRoutingSettings();
        TransportRouter transport_router(settings);
        transport_router.SetGraph(DeserializeEdgesFromGraph(), DeserializeIncidenceListsFromGraph());
        transport_router.SetRouter(DeserializeRouter());
        transport_router.SetStopAsPairNumber(DeserializeStopAsPairNumber(transport_catalogue));
        transport_router.SetEdgeidToType(DeserializeEdgeidToType(transport_catalogue));
        return transport_router;
}

void SerialHandler::DeserializeStops(transport_catalogue::TransportCatalogue& transport_catalogue) {

    for (size_t i = 0; i < transport_catalogue_proto_.stops_size(); ++i) {
        const transport_catalogue_serialize::Stop& stop_proto = transport_catalogue_proto_.stops(i);
        transport_catalogue::stop::Stop stop;
        stop.name = stop_proto.name();
        stop.coordinates = {stop_proto.coordinates().lat(), stop_proto.coordinates().lng()};
        stops_.emplace_back(stop);

    }
    for (auto& stop : stops_) {
        transport_catalogue.AddStop(stop.name, &stop);
    }
}

void SerialHandler::DeserializeBuses(transport_catalogue::TransportCatalogue& transport_catalogue) {
    for (size_t i = 0; i < transport_catalogue_proto_.distance_between_stops_size(); ++i) {
        const transport_catalogue_serialize::DistanceBetweenStops& distance_between_stops_proto = transport_catalogue_proto_.distance_between_stops(i);
        transport_catalogue.AddDistanceBetweenStops(transport_catalogue.FindStop(distance_between_stops_proto.stop_from()),
                                                 transport_catalogue.FindStop(distance_between_stops_proto.stop_to()),
                                                 distance_between_stops_proto.distance());
    }
}

void SerialHandler::DeserializeDistanceBetweenStops(transport_catalogue::TransportCatalogue& transport_catalogue) {
    for (size_t i = 0; i < transport_catalogue_proto_.buses_size(); ++i) {
        const transport_catalogue_serialize::Bus bus_proto = transport_catalogue_proto_.buses(i);
        transport_catalogue::bus::Bus bus;
        bus.name = bus_proto.name();
        bus.circle = bus_proto.is_circle();
        for (size_t j = 0; j < bus_proto.route_size(); ++j) {
            bus.route.push_back(transport_catalogue.FindStop(bus_proto.route(j)));
        }
        buses_.emplace_back(bus);
    }
    for (auto& bus: buses_) {
        transport_catalogue.AddBus(bus.name, &bus);
    }

}

MapRenderSettings SerialHandler::DeserializeMapRenderSettings() {
    MapRenderSettings render_settings;
    const map_renderer_proto::RenderSettings& render_settings_proto = transport_catalogue_proto_.map_renderer();
    render_settings.width = render_settings_proto.width();
    render_settings.height = render_settings_proto.height();
    render_settings.padding = render_settings_proto.padding();
    render_settings.line_width = render_settings_proto.line_width();
    render_settings.stop_radius = render_settings_proto.stop_radius();
    render_settings.bus_label_font_size = render_settings_proto.bus_label_font_size();
    for (size_t i = 0; i < render_settings_proto.bus_label_offset_size(); ++i) {
        render_settings.bus_label_offset.push_back(render_settings_proto.bus_label_offset(i));
    }
    render_settings.stop_label_font_size = render_settings_proto.stop_label_font_size();
    for (size_t i = 0; i < render_settings_proto.stop_label_offset_size(); ++i) {
        render_settings.stop_label_offset.push_back(render_settings_proto.stop_label_offset(i));
    }
    render_settings.underlayer_color = DeserializeColor(render_settings_proto.underlayer_color());
    render_settings.underlayer_width = render_settings_proto.underlayer_width();
    for (size_t i = 0; i < render_settings_proto.color_palette_size(); ++i) {
        svg::Color color = DeserializeColor(render_settings_proto.color_palette(i));
        render_settings.color_palette.push_back(color);
    }
    return render_settings;
}

svg::Color SerialHandler::DeserializeColor(const svg_proto::Color& color_proto) {
    if (color_proto.is_string()) {
        return color_proto.color();
    } else if (color_proto.is_rgb()) {
        svg::Rgb rgb {static_cast<uint8_t>(color_proto.red()),
                      static_cast<uint8_t>(color_proto.green()),
                      static_cast<uint8_t>(color_proto.blue())};
        return rgb;
    } else {
        svg::Rgba rgba {static_cast<uint8_t>(color_proto.red()),
                        static_cast<uint8_t>(color_proto.green()),
                        static_cast<uint8_t>(color_proto.blue()),
                        color_proto.opacity()};
        return rgba;
    }
}

transport_catalogue::RoutingSettings SerialHandler::DeserializeRoutingSettings() {
    transport_catalogue::RoutingSettings routing_settings;
    const transport_router_proto::RoutingSettings& routing_settings_proto = transport_catalogue_proto_.transport_router().routing_settings();
    routing_settings.bus_velocity_ = routing_settings_proto.bus_velocity();
    routing_settings.bus_wait_time_ = routing_settings_proto.bus_wait_time();
    return routing_settings;
}

std::vector<graph::Edge<double>> SerialHandler::DeserializeEdgesFromGraph() {
    const graph_proto::DirectedWeightedGraph& graph_proto = transport_catalogue_proto_.transport_router().graph();
    std::vector<graph::Edge<double>> edges(graph_proto.edges_size());
    for (size_t i = 0; i < graph_proto.edges_size(); ++i) {
        const graph_proto::Edge& edge_proto = graph_proto.edges(i);
        edges[i] = {edge_proto.from_edge(), edge_proto.to_edge(), edge_proto.weight()};
    }
    return edges;
}

std::vector<graph::DirectedWeightedGraph<double>::IncidenceList> SerialHandler::DeserializeIncidenceListsFromGraph() {
    const graph_proto::DirectedWeightedGraph& graph_proto = transport_catalogue_proto_.transport_router().graph();
    std::vector<graph::DirectedWeightedGraph<double>::IncidenceList> incidence_lists(graph_proto.incidence_lists_size());
    for (size_t i = 0; i < graph_proto.incidence_lists_size(); ++i) {
        const graph_proto::IncidenceList& incidence_list_proto = graph_proto.incidence_lists(i);
        for (size_t j = 0; j < incidence_list_proto.edge_id_size(); ++j) {
            incidence_lists[i].push_back(incidence_list_proto.edge_id(j));
        }
    }
    return incidence_lists;
}

graph::Router<double>::RoutesInternalData SerialHandler::DeserializeRouter() {
    const graph_proto::Router router_proto = transport_catalogue_proto_.transport_router().router();
    graph::Router<double>::RoutesInternalData routes_internal_data(router_proto.routes_internal_data_size());
    for (size_t i = 0; i < router_proto.routes_internal_data_size(); ++i) {
        const graph_proto::RoutesInternalData routes_internal_data_proto = router_proto.routes_internal_data(i);
        for (size_t j = 0; j < routes_internal_data_proto.route_internal_data_size(); ++j) {
            const graph_proto::RouteInternalData& route_internal_data_proto = routes_internal_data_proto.route_internal_data(j);
            graph::Router<double>::RouteInternalData route_internal_data;
            if (route_internal_data_proto.has_data()) {
                route_internal_data.weight = route_internal_data_proto.weight();
                if (route_internal_data_proto.has_prev_edge()) {
                    route_internal_data.prev_edge = route_internal_data_proto.prev_edge();
                }
                routes_internal_data[i].push_back(move(route_internal_data));
            } else {
                routes_internal_data[i].push_back(nullopt);
            }
        }
    }
    return routes_internal_data;
}

std::unordered_map<transport_catalogue::stop::Stop*, StopPairVertexId> SerialHandler::DeserializeStopAsPairNumber(
        TransportCatalogue transport_catalogue) {
    const auto& stop_as_pair_number_proto = transport_catalogue_proto_.transport_router().stop_id_to_pair_vertex_id();
    std::unordered_map<transport_catalogue::stop::Stop*, StopPairVertexId> stop_as_pair_number(stop_as_pair_number_proto.size());
    for (const auto& [stop, vertex] : stop_as_pair_number_proto) {
        stop_as_pair_number[transport_catalogue.FindStop(stop)] = {vertex.bus_wait_begin(), vertex.bus_wait_end()};
    }
    return stop_as_pair_number;
}

std::unordered_map<graph::EdgeId, EdgeInfo> SerialHandler::DeserializeEdgeidToType(
        TransportCatalogue transport_catalogue) {
    const auto& edge_id_to_type_proto = transport_catalogue_proto_.transport_router().edge_id_to_type();
    std::unordered_map<graph::EdgeId, EdgeInfo> edge_id_to_type(edge_id_to_type_proto.size());
    for (const auto& [id, info] : edge_id_to_type_proto) {
        if (info.has_wait_edge_info()) {
            const transport_router_proto::WaitEdgeInfo wait_edge_proto = info.wait_edge_info();
            WaitEdgeInfo wait_edge = {transport_catalogue.FindStop(wait_edge_proto.stop_name())->name, wait_edge_proto.minutes()};
            edge_id_to_type[id] = move(wait_edge);

        } else {
            const transport_router_proto::BusEdgeInfo& bus_edge_proto = info.bus_edge_info();
            BusEdgeInfo bus_edge = {transport_catalogue.GetBusInfo(bus_edge_proto.bus_name())->name, bus_edge_proto.span_count(), bus_edge_proto.time()};
            edge_id_to_type[id] = move(bus_edge);
        }
    }
    return edge_id_to_type;
}

} // namespace serial_handler