#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <unordered_map>
#include <functional>

#include "transport_catalogue.h"

namespace catalogue {

	namespace input_reader {
		using InfoMain = std::vector<Stop>;
		using InfoSecondary = std::unordered_map<std::string, std::string>;

		std::string ReadLine(std::istream& input);

		std::vector<std::string> ReadInputData(std::istream& input);

		void GenerationDatabase(TransportCatalogue& catalogue, std::istream& input);

		std::pair<InfoMain, InfoSecondary> SplitQueryStopOnMainAndSecondary(
			const std::vector<std::string>& all_queries);

		std::pair<std::string_view, std::string_view> Split(std::string_view line, char symbol);

		Bus ParseRoute(const TransportCatalogue& catalogue, std::string_view route, char symbol);

		std::deque<const Stop*> SplitStopsInRoute(const TransportCatalogue& catalogue,
			std::string_view& stops, char symbol);

		std::unordered_map<std::string, size_t> ParseLine(std::string_view line);

		std::vector<Bus> CutBuses(const TransportCatalogue& catalogue,
			const std::vector<std::string>& all_queries);

		Bus GenerateInfoAboutBus(const TransportCatalogue& catalogue,
			const std::string& query_bus);
	}

}
