#pragma once
#include <iostream>

#include "transport_catalogue.h"

namespace input_reader {
    class InputReader {
    public:
        explicit InputReader(transport_catalogue::TransportCatalogue& catalogue) : catalogue_(catalogue) {}

        struct Line {
            std::string name;
            std::string type;
            std::string info;
        };

        struct Distance {
            std::string stop_1;
            std::string stop_2;
            int distance;
        };

        void LoadData(size_t count);

        void AddDistance(std::string& stop_name, std::string& info);

        void ProcessStops(Line& line);

        void ProcessBus(Line& line);

    private:
        std::deque<Line> lines_;
        transport_catalogue::TransportCatalogue &catalogue_;
        std::deque<transport_catalogue::stop::Stop> stops_;
        std::deque<transport_catalogue::bus::Bus> buses_;
        std::deque<Distance> distance_between_stops_;
    };

}
