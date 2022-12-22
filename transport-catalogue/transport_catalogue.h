#pragma once
#include <deque>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <set>

#include "geo.h"

namespace transport_catalogue {
    namespace stop {

        struct Stop {
            std::string name;
            Coordinates coordinates;
        };
        namespace hash {
            struct Hash {
                size_t operator() (const std::pair<stop::Stop*, stop::Stop*>& stops) const noexcept {
                    std::size_t h1 = std::hash<std::uint32_t>{}(reinterpret_cast<size_t>(stops.first));
                    std::size_t h2 = std::hash<std::uint32_t>{}(reinterpret_cast<size_t>(stops.second));
                    return h1 ^ (h2 << 1);
                }
            };
        }
    }

    namespace bus {

        struct Bus {
            std::string name;
            std::vector<stop::Stop*> route;
            bool circle = false;
        };
    }

    struct RoutingSettings {
        int bus_wait_time_;
        double bus_velocity_;
    };

    class TransportCatalogue {
        using stop_stop_to_distance = std::unordered_map<std::pair<stop::Stop*, stop::Stop*>, uint32_t, stop::hash::Hash>;
    public:
        int GetDistanceBetween (stop::Stop* from, stop::Stop* to);

        void AddStop(std::string_view name, stop::Stop* stop);

        stop::Stop* FindStop(std::string_view name);

        void AddBus(std::string& name, bus::Bus* bus);

        bool FindBus(std::string_view name);

        bus::Bus* GetBusInfo(std::string_view name);

        double GetRouteLength(std::string_view name);

        void AddDistanceBetweenStops(stop::Stop* from, stop::Stop* to, int distance);

        std::set<std::string_view>& GetBuses(stop::Stop* stop);

        std::unordered_map<std::string_view, bus::Bus*>& GetAllBuses();

        std::unordered_map<std::string_view, stop::Stop*>& GetAllStops();

        void SetRoutingSettings(RoutingSettings settings);

        RoutingSettings& GetRoutingSettings();
    private:
        std::unordered_map<stop::Stop*, std::set<std::string_view>> buses_to_stop_;
        std::unordered_map<std::string_view, bus::Bus*> stop_to_bus_;
        std::unordered_map<std::string_view, stop::Stop*> stops_;
        stop_stop_to_distance distance_between_stops_;
        RoutingSettings routing_settings_;
    };
}