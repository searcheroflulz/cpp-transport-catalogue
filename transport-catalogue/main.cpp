#include <fstream>
#include <iostream>
#include <string_view>
#include <sstream>

#include "map_renderer.h"
#include "json_reader.h"
#include "json.h"
#include "transport_catalogue.h"
#include "serialization.h"
#include "transport_router.h"


using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        transport_catalogue::TransportCatalogue transport_catalogue;
        transport_router::TransportRouter router(transport_catalogue.GetRoutingSettings());
        json_reader::JsonReader json_reader(transport_catalogue, json::Load(std::cin).GetRoot(), &router);
        router.BuildTransportRouter(transport_catalogue);

        serial_handler::SerialHandler serial_handler(json_reader.GetSerializationSettings());
        std::ofstream output(serial_handler.GetSettings().name_file, std::ios::binary);
        serial_handler.SerializationTransportCatalogue(transport_catalogue);
        serial_handler.SerializationMapRenderSettings(json_reader.GetMapRenderSettings());
        serial_handler.SerializationTransportRouter(router);
        serial_handler.Serialization(output);
        output.close();
    } else if (mode == "process_requests"sv) {
        json_reader::JsonReader json_reader(json::Load(std::cin).GetRoot());
        serial_handler::SerialHandler serial_handler(move(json_reader.GetSerializationSettings()));
        std::ifstream input(serial_handler.GetSettings().name_file, std::ios::binary);
        serial_handler.Deserialize(input);
        transport_catalogue::TransportCatalogue transport_catalogue = serial_handler.GetTransportCatalogue();
        transport_router::TransportRouter transport_router = serial_handler.GetTransportRouter(transport_catalogue);
        auto temp = transport_router.GetRoutingSettings();
        map_renderer::MapRenderer map_renderer(move(serial_handler.GetMapRenderSettings()));
        map_renderer.RenderMap(transport_catalogue);
        json_reader.SetMap(map_renderer.GetMapAsString());
        transport_router.GetRoutingSettings() = temp;
        json_reader.SetTransportRouter(&transport_router);
        json_reader.OutputRequest(&transport_catalogue);

    } else {
        PrintUsage();
        return 1;
    }
}