#include "stat_reader.h"
#include "input_reader.h"

#include <iomanip>

namespace catalogue::stat_reader {

	void SearchInfo(const TransportCatalogue& catalogue, std::istream& input) {
		// 1. Read queries 
		std::vector<std::string> unprocessed_requests = std::move(input_reader::ReadInputData(input));

		// 2. collecting a vector of answers
		for (const std::string& query : unprocessed_requests) {

			if (query[0] == 'B') {
				std::string_view name = CutName(query);
				BusInfo bus_info = catalogue.GetBusInfo(name);
				PrintInfoBus(bus_info);
			}
			else {
				std::string_view name = CutName(query);
				StopInfo stop_info = catalogue.GetStopInfo(name);
				PrintInfoStop(stop_info);
			}
		}
	}

	std::string_view CutName(std::string_view unprocessed_request) {

		std::string_view name = (unprocessed_request[0] == 'B')
			? unprocessed_request.substr(3)
			: unprocessed_request.substr(4);
		size_t begin = name.find_first_not_of(' ');
		size_t end = name.find_last_not_of(' ');
		name = name.substr(begin, end + 1 - begin);

		return name;
	}

	void PrintInfoBus(const BusInfo& bus) {
		if (bus.bus_is_existing) {
			std::cout << "Bus " << bus.name << ": "
				<< bus.number_stops << " stops on route, "
				<< bus.unique_stops << " unique stops, "
				<< std::setprecision(6)
				<< bus.length_route << " route length, "
				<< bus.curvature << " curvature" << std::endl;
		}
		else {
			std::cout << "Bus " << bus.name << ": not found" << std::endl;
		}
	}

	void PrintInfoStop(const StopInfo& stop_info) {
		if (!stop_info.to_exist) {
			std::cout << "Stop " << stop_info.name << ": not found" << std::endl;
		}
		else if (stop_info.to_exist && stop_info.buses.empty()) {
			std::cout << "Stop " << stop_info.name << ": no buses" << std::endl;
		}
		else {
			std::cout << "Stop " << stop_info.name << ": buses ";

			std::string str;
			for (std::string_view bus : stop_info.buses) {
				str += std::string(bus) + " ";
			}
			str = str.substr(0, str.find_last_not_of(' ') + 1);

			std::cout << str << std::endl;
		}
	}

}
