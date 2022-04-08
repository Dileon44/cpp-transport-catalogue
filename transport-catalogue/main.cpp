#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <iostream>

int main() {
	catalogue::TransportCatalogue catalogue;
	catalogue::input_reader::GenerationDatabase(catalogue, std::cin);
	catalogue::stat_reader::SearchInfo(catalogue, std::cin);

	return 0;
}
