#include "stat_reader.h"


#include "transport_catalogue.h"

namespace stat_reader {
    void BusOutput(std::string_view name, transport_catalogue::TransportCatalogue& catalogue) {
        auto bus = catalogue.GetBusInfo(name);
        uint32_t route_length_int = 0;
        size_t stops_on_route = 0;
        size_t unique_stops_count = 0;
        double route_length = 0;
        if (bus != nullptr) {
            bus->circle ? stops_on_route = bus->route.size() : stops_on_route = bus->route.size() + bus->route.size() - 1;
            std::unordered_set<transport_catalogue::stop::Stop*> unique_stops = {bus->route.begin(), bus->route.end()};
            unique_stops_count = unique_stops.size();
            route_length = catalogue.GetRouteLength(bus->name);
        } else {
            std::cout << "Bus " << name << ": " << "not found" << std::endl;
            return;
        }

        for (int i = 0; i != bus->route.size() - 1; i++) {
            route_length_int += catalogue.GetDistanceBetween(bus->route[i], bus->route[i + 1]);
        }
        if (!bus->circle) {
            for (size_t i = bus->route.size(); i != 1; i--) {
                route_length_int += catalogue.GetDistanceBetween(bus->route[i - 1], bus->route[i - 2]);
            }
        }
        double curvature = route_length_int / route_length;
        std::cout << "Bus " << name << ": " << stops_on_route << " stops on route, " << unique_stops_count << " unique stops, " << route_length_int << " route length, " << curvature << " curvature" << std::endl;
    }

    void StopOutput(std::string_view name, transport_catalogue::TransportCatalogue& catalogue) {
        auto ptr = catalogue.FindStop(name);
        if (ptr != nullptr) {
            if (catalogue.GetBuses(ptr).empty()) {
                std::cout << "Stop " << name << ": no buses" << std::endl;
                return;
            }
            std::set<std::string_view> buses = catalogue.GetBuses(ptr);
            std::string result;
            for (auto& bus: buses) {
                result += bus;
                result.push_back(' ');
            }
            std::cout << "Stop " << name << ": buses " << result << std::endl;
            return;
        }
        std::cout << "Stop " << name << ": not found" << std::endl;
    }

    void ReadStat(transport_catalogue::TransportCatalogue& catalogue, size_t count) {
        std::vector<std::pair<std::string, std::string>> lines;
        std::vector<std::string> result;
        for (int i = 0; i != count; i++) {
            std::string line;
            std::pair<std::string, std::string> type_and_name;
            std::getline(std::cin, line);
            size_t space = line.find_first_of(' ');
            std::string type = line.substr(0, space);
            std::string name = line.substr(space + 1, line.npos);
            lines.emplace_back(type, name);
        }
        for (auto& line: lines) {
            if (line.first == "Bus") {
                BusOutput(line.second, catalogue);
            }
            else {
                StopOutput(line.second, catalogue);
            }
        }
    }
}