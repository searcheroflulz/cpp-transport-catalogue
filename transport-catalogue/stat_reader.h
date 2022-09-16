#pragma once
#include <iostream>
#include <unordered_set>
#include <set>

#include "transport_catalogue.h"

namespace stat_reader {
    void BusOutput(std::string_view name, transport_catalogue::TransportCatalogue& catalogue);

    void StopOutput(std::string_view name, transport_catalogue::TransportCatalogue& catalogue);

    void ReadStat(transport_catalogue::TransportCatalogue& catalogue, size_t count);
}