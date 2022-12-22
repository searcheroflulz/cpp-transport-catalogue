#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "unordered_set"
#include <variant>
#include "domain.h"


/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
namespace json_reader {
    class JsonReader {
    public:
        struct Distance {
            std::string stop_1;
            std::string stop_2;
            int distance;
        };

        JsonReader(transport_catalogue::TransportCatalogue& catalogue, const json::Node& document) : catalogue_(catalogue), document_(document) {
            LoadDataFromJson();
        }

        svg::Color LoadColor(const json::Node& data);

        void LoadRenderSettings(const std::map<std::string, json::Node>& info);

        void LoadRoutingSettings(const std::map<std::string, json::Node>& info);

        void LoadDataFromJson();

        void InputDataToCatalogue(const std::vector<json::Node>& info);

        void LoadStops(const std::map<std::string, json::Node>& document);

        void LoadBuses(const std::map<std::string, json::Node>& document);

        void BuildJsonBus(json::Builder& builder, const std::map<std::string, json::Node>& document);

        void BuildJsonStop(json::Builder& builder, const std::map<std::string, json::Node>& document);

        void OutputRequest(const std::vector<json::Node>& info);

        void BuildJsonRoute(json::Builder& builder, const RouteRequest& request, const transport_catalogue::TransportCatalogue& catalogue);

    private:
        transport_catalogue::TransportCatalogue& catalogue_;
        std::deque<transport_catalogue::stop::Stop> stops_;
        std::deque<Distance> distance_between_stops_;
        const json::Node& document_;
        std::deque<transport_catalogue::bus::Bus> buses_;
        map_renderer::MapRenderSettings settings_;
        std::string map_;
    };
}