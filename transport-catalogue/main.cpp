#include <iostream>
#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    size_t count_input_data = 0;
    size_t count_output_data = 0;
    std::cin >> count_input_data;
    std::cin.ignore(32767, '\n');

    input_reader::InputReader reader(catalogue);
    reader.LoadData(count_input_data);

    std::cin >> count_output_data;
    std::cin.ignore(32767, '\n');
    stat_reader::ReadStat(catalogue, count_output_data);

}