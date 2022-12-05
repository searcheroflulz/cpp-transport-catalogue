#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "unordered_set"
#include <variant>


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

        void LoadDataFromJson();

        void InputDataToCatalogue(const std::vector<json::Node>& info);

        void LoadStops(const std::map<std::string, json::Node>& document);

        void LoadBuses(const std::map<std::string, json::Node>& document);

        json::Node BuildJsonBus(const std::map<std::string, json::Node>& document);

        json::Node BuildJsonStop(const std::map<std::string, json::Node>& document);

        void OutputRequest(const std::vector<json::Node>& info);

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