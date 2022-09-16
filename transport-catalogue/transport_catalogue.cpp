#include "transport_catalogue.h"

namespace transport_catalogue {
    int TransportCatalogue::GetDistanceBetween (stop::Stop* from, stop::Stop* to) {
        if (!distance_between_stops_.count(std::pair{from, to})) {
            return distance_between_stops_.at(std::pair{to, from});
        }
        return distance_between_stops_.at(std::pair{from, to});
    }

    void TransportCatalogue::AddStop(std::string_view name, stop::Stop* stop) {
        stops_[name] = stop;
        buses_to_stop_[stop];
    }

    stop::Stop* TransportCatalogue::FindStop(std::string_view name) {
        static stop::Stop* temp;
        if (stops_.count(name))
            return stops_.at(name);
        return temp;
    }

    void TransportCatalogue::AddBus(std::string& name, bus::Bus* bus) {
        stop_to_bus_[name] = bus;
        for (auto& stop: bus->route) {
            if (buses_to_stop_.count(stop))
                buses_to_stop_[stop].insert(bus->name);
        }
    }

    bool TransportCatalogue::FindBus(std::string_view name) {
        return stop_to_bus_.count(name);
    }

    bus::Bus* TransportCatalogue::GetBusInfo(std::string_view name) {
        if (FindBus(name)) {
            return stop_to_bus_.at(name);
        }
        return nullptr;
    }

    double TransportCatalogue::GetRouteLength(std::string_view name) {
        double result = 0;
        if (GetBusInfo(name)->circle) {
            for (size_t i = 0; i != stop_to_bus_.at(name)->route.size() - 1; i++) {
                result += ComputeDistance(stop_to_bus_.at(name)->route[i]->coordinates,
                                          stop_to_bus_.at(name)->route[i + 1]->coordinates);
            }
        } else {
            for (size_t i = 0; i != stop_to_bus_.at(name)->route.size() - 1; i++) {
                result += ComputeDistance(stop_to_bus_.at(name)->route[i]->coordinates,
                                          stop_to_bus_.at(name)->route[i + 1]->coordinates);
            }
            for (size_t i = stop_to_bus_.at(name)->route.size(); i != 1; i--) {
                result += ComputeDistance(stop_to_bus_.at(name)->route[i - 1]->coordinates,
                                          stop_to_bus_.at(name)->route[i - 2]->coordinates);
            }
        }
        return result;
    }

    void TransportCatalogue::AddDistanceBetweenStops(stop::Stop* from, stop::Stop* to, int distance) {
        distance_between_stops_[{from, to}] = distance;
    }

    std::set<std::string_view>& TransportCatalogue::GetBuses(stop::Stop* stop) {
        return buses_to_stop_.at(stop);
    }
}