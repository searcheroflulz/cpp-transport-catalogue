#include <iostream>
#include "transport_catalogue.h"
#include "json.h"
#include "json_reader.h"

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    json_reader::JsonReader(catalogue, json::Load(std::cin).GetRoot());
}