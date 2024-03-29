#pragma once
#include <string>
#include <variant>
#include "graph.h"


struct RouteRequest {
    std::string from;
    std::string to;
    int id;
};

struct StopPairVertexId {
    graph::VertexId bus_wait_begin;
    graph::VertexId bus_wait_end;
};

struct WaitEdgeInfo {
    std::string_view stop_name;
    double time = 0;
};

struct BusEdgeInfo {
    std::string_view bus_name;
    size_t span_count = 0;
    double time = 0;
};

using EdgeInfo = std::variant<WaitEdgeInfo, BusEdgeInfo>;

struct RouteInfo {
    double total_time = 0;
    std::vector<std::variant<WaitEdgeInfo, BusEdgeInfo>> edges;
};

struct SerializationSettings {
    std::string name_file;
};
