#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_reader {
    svg::Color JsonReader::LoadColor(const json::Node& data) {
        svg::Color result;
        if (data.IsString())
            return svg::Color{data.AsString()};
        if (data.IsArray()) {
            if (data.AsArray().size() == 3) {
                svg::Rgb rgb;
                rgb.red = data.AsArray()[0].AsInt();
                rgb.green = data.AsArray()[1].AsInt();
                rgb.blue = data.AsArray()[2].AsInt();
                return svg::Color{rgb};
            } else {
                svg::Rgba rgba;
                rgba.red = data.AsArray()[0].AsInt();
                rgba.green = data.AsArray()[1].AsInt();
                rgba.blue = data.AsArray()[2].AsInt();
                rgba.opacity = data.AsArray()[3].AsDouble();
                return svg::Color{rgba};
            }
        }
        return {};
    }

    void JsonReader::LoadRenderSettings(const std::map<std::string, json::Node>& info) {
        settings_.width = info.at("width").AsDouble();
        settings_.height = info.at("height").AsDouble();
        settings_.padding = info.at("padding").AsDouble();
        settings_.stop_radius = info.at("stop_radius").AsDouble();
        settings_.line_width = info.at("line_width").AsDouble();
        settings_.bus_label_font_size = info.at("bus_label_font_size").AsInt();
        for (auto& setting: info.at("bus_label_offset").AsArray()) {
            settings_.bus_label_offset.push_back(setting.AsDouble());
        }
        settings_.stop_label_font_size = info.at("stop_label_font_size").AsInt();
        for (auto& setting: info.at("stop_label_offset").AsArray()) {
            settings_.stop_label_offset.push_back(setting.AsDouble());
        }
        settings_.underlayer_color = LoadColor(info.at("underlayer_color"));
        settings_.underlayer_width = info.at("underlayer_width").AsDouble();
        for (auto& color: info.at("color_palette").AsArray()) {
            settings_.color_palette.push_back(LoadColor(color));
        }
    }

    void JsonReader::LoadDataFromJson() {
        auto asmap = document_.AsMap();
        for (auto& temp: asmap) {
            if (temp.first == "base_requests") {
                auto base_request = temp.second.AsArray();
                InputDataToCatalogue(base_request);
            }
            if (temp.first == "render_settings") {
                auto render_settings = temp.second.AsMap();
                LoadRenderSettings(render_settings);
                map_renderer::MapRenderer renderer(settings_);
                renderer.LoadBuses(catalogue_.GetAllBuses());
                renderer.LoadCoordinates(catalogue_);
                map_ = renderer.Output();
            }
            if (temp.first == "stat_requests") {
                auto stat_requests = temp.second.AsArray();
                OutputRequest(stat_requests);
            }
        }
    }

    void JsonReader::InputDataToCatalogue(const std::vector<json::Node>& info) {
        std::deque<std::map<std::string, json::Node>const *> buses;
        for (auto& request: info) {
            if (request.AsMap().at("type").AsString() == "Stop") {
                LoadStops(request.AsMap());
            }
            if (request.AsMap().at("type").AsString() == "Bus") {
                buses.push_back(&request.AsMap());
            }
        }
        for (auto& stop: stops_) {
            catalogue_.AddStop(stop.name, &stop);
        }
        for (auto& stop: distance_between_stops_) {
            catalogue_.AddDistanceBetweenStops(catalogue_.FindStop(stop.stop_1), catalogue_.FindStop(stop.stop_2), stop.distance);
        }
        for (auto& bus: buses) {
            LoadBuses(*bus);
        }
    }

    void JsonReader::LoadStops(const std::map<std::string, json::Node>& document) {
        transport_catalogue::stop::Stop stop;
        stop.name = document.at("name").AsString();
        stop.coordinates.lat = document.at("latitude").AsDouble();
        stop.coordinates.lng = document.at("longitude").AsDouble();
        stops_.emplace_back(stop);
        const auto& road_distances = document.at("road_distances").AsMap();
        for (const auto& distance : road_distances) {
            Distance distance_between_stop;
            distance_between_stop.stop_1 = stop.name;
            distance_between_stop.stop_2 = distance.first;
            distance_between_stop.distance = distance.second.AsInt();
            distance_between_stops_.emplace_back(distance_between_stop);
        }
    }

    void JsonReader::LoadBuses(const std::map<std::string, json::Node>& document) {
        const json::Array& stops = document.at("stops").AsArray();
        transport_catalogue::bus::Bus bus;
        bus.name = document.at("name").AsString();
        bus.circle = document.at("is_roundtrip").AsBool();
        for (const auto& stop: stops) {
            bus.route.emplace_back(catalogue_.FindStop(stop.AsString()));
        }
        auto* ptr = &buses_.emplace_back(bus);
        catalogue_.AddBus(ptr->name, ptr);
    }

    void JsonReader::BuildJsonBus(json::Builder& builder, const std::map<std::string, json::Node>& document) {
        uint32_t route_length_int = 0;
        size_t stops_on_route = 0;
        size_t unique_stops_count = 0;
        auto bus = catalogue_.GetBusInfo(document.at("name").AsString());

        double route_length = 0;
        if (bus != nullptr) {
            bus->circle ? stops_on_route = bus->route.size() : stops_on_route = bus->route.size() + bus->route.size() - 1;
            std::unordered_set<transport_catalogue::stop::Stop*> unique_stops = {bus->route.begin(), bus->route.end()};
            unique_stops_count = unique_stops.size();
            route_length = catalogue_.GetRouteLength(bus->name);
        } else {
            builder.StartDict().Key("request_id").Value(document.at("id").AsInt()).Key("error_message").Value(std::string{"not found"}).EndDict();
            return;
        }

        for (size_t i = 0; i != bus->route.size() - 1; i++) {
            route_length_int += catalogue_.GetDistanceBetween(bus->route[i], bus->route[i + 1]);
        }
        if (!bus->circle) {
            for (size_t i = bus->route.size(); i != 1; i--) {
                route_length_int += catalogue_.GetDistanceBetween(bus->route[i - 1], bus->route[i - 2]);
            }
        }
        double curvature = route_length_int / route_length;
        builder.StartDict();
        builder.Key("curvature").Value(curvature)
        .Key("request_id").Value(document.at("id").AsInt())
        .Key("route_length").Value(static_cast<int>(route_length_int))
        .Key("stop_count").Value(static_cast<int>(stops_on_route))
        .Key("unique_stop_count").Value(static_cast<int>(unique_stops_count)).EndDict();
    }

    void JsonReader::BuildJsonStop(json::Builder& builder, const std::map<std::string, json::Node>& document) {
        auto ptr = catalogue_.FindStop(document.at("name").AsString());
        if (ptr != nullptr) {
            std::vector<std::string> buses_vect;
            if (catalogue_.GetBuses(ptr).empty()) {
                builder.StartDict().Key("buses").StartArray();
                for (const auto& bus: buses_vect) {
                    builder.Value(bus);
                }
                builder.EndArray().Key("request_id").Value(document.at("id").AsInt()).EndDict();
                return;
            }
            std::set<std::string_view> buses = catalogue_.GetBuses(ptr);
            for (auto& bus: buses) {
                buses_vect.emplace_back(bus);
            }
            builder.StartDict().Key("buses").StartArray();
            for (const auto& bus: buses_vect) {
                builder.Value(bus);
            }
            builder.EndArray().Key("request_id").Value(document.at("id").AsInt()).EndDict();
            return;
        }
        builder.StartDict().Key("request_id").Value(document.at("id").AsInt()).Key("error_message").Value(std::string{"not found"}).EndDict();
    }

    void JsonReader::OutputRequest(const std::vector<json::Node>& info) {
        json::Builder builder;
        builder.StartArray();
        if (info.empty()) {
            return;
        }
        for (auto& request: info) {
            if (request.AsMap().at("type").AsString() == "Map") {
                builder.StartDict().Key("map").Value(map_).Key("request_id").Value(request.AsMap().at("id").AsInt()).EndDict();
            }
            if (request.AsMap().at("type").AsString() == "Stop") {
                BuildJsonStop(builder, request.AsMap());
            }
            if (request.AsMap().at("type").AsString() == "Bus") {
                BuildJsonBus(builder, request.AsMap());
            }
        }
        builder.EndArray();
        json::PrintNode(builder.Build(), json::PrintContext{std::cout});
    }
}