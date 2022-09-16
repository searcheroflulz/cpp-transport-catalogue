#include "input_reader.h"

namespace input_reader {
    void InputReader::LoadData(size_t count) {
        for (size_t i = 0; i != count; i++) {
            std::string name;
            std::string type;
            std::string info;
            std::string line;
            std::getline(std::cin, line);
            auto begin = line.find_first_of(' ');
            auto end = line.find_first_of(':');
            name = line.substr(begin + 1, end - begin - 1);
            type = line.substr(0, begin);
            info = line.substr(end + 2, line.size() - end);
            lines_.emplace_back(Line{name, type, info});
        }
        for (auto& line: lines_) {
            if (line.type == "Stop")
                ProcessStops(line);
        }
        for (auto& stop: stops_) {
            catalogue_.AddStop(stop.name, &stop);
        }
        for (auto& stop: distance_between_stops_) {
            catalogue_.AddDistanceBetweenStops(catalogue_.FindStop(stop.stop_1), catalogue_.FindStop(stop.stop_2), stop.distance);
        }
        for (auto& line: lines_) {
            if (line.type == "Bus")
                ProcessBus(line);
        }
    }

    void InputReader::AddDistance(std::string& stop_name, std::string& info) {
        size_t next = 0;
        while (next != std::string::npos) {
            next = info.find_first_of(',');
            std::string stop_2 = info.substr(info.find_first_of(' ') + 4, next);
            stop_2 = stop_2.substr(0, stop_2.find_first_of(','));
            distance_between_stops_.emplace_back(InputReader::Distance{stop_name, stop_2, std::stoi(info.substr(0, info.find_first_of(' ') - 1))});
            info = info.substr(next + 2, std::string::npos);
        }
    }

    void InputReader::ProcessStops(Line& line) {
        transport_catalogue::stop::Stop stop;
        stop.name = line.name;
        auto first_end = line.info.find_first_of(',');
        std::string lat = line.info.substr(0, first_end);
        stop.coordinates.lat = std::stod(lat);
        std::string lng = line.info.substr(first_end + 2, line.info.size());
        std::string distance = lng;
        lng = lng.substr(0, lng.find_first_of(','));
        stop.coordinates.lng = std::stod(lng);
        stops_.emplace_back(stop);
        if (distance.find_first_of(',') > distance.size())
            return;
        distance = distance.substr(distance.find_first_of(',') + 2, distance.npos);
        AddDistance(stop.name, distance);
    }

    void InputReader::ProcessBus(Line& line) {
        std::deque<std::string> stops;
        transport_catalogue::bus::Bus bus;
        bus.name = line.name;
        char symbol = '-';
        if (line.info.find('>') < line.info.size()) {
            bus.circle = true;
            symbol = '>';
        }
        while(!line.info.empty()) {
            auto space = line.info.find_first_of(symbol);
            if (space > line.info.size()) {
                stops.emplace_back(line.info);
                break;
            }
            stops.emplace_back(line.info.substr(0, space - 1));
            line.info = line.info.substr(space + 2, line.info.npos);
        }
        for (auto& stop: stops) {
            bus.route.emplace_back(catalogue_.FindStop(stop));
        }
        auto* ptr = &buses_.emplace_back(bus);
        catalogue_.AddBus(ptr->name, ptr);
    }
}