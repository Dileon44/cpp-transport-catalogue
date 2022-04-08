#include "input_reader.h"

namespace catalogue::input_reader {

	std::string ReadLine(std::istream& input) {
		std::string str;
		std::getline(input, str);

		return str;
	}

	std::vector<std::string> ReadInputData(std::istream& input) {
		size_t number_requaests = std::stoi(ReadLine(input));

		std::vector<std::string> queries;
		queries.reserve(number_requaests);

		for (size_t count = 1; count <= number_requaests; ++count) {
			std::string line = std::move(ReadLine(input));
			// удаляем пробелы слева
			line = line.substr(line.find_first_not_of(' '));
			queries.push_back(std::move(line));
		}

		return queries;
	}

	void GenerationDatabase(TransportCatalogue& catalogue, std::istream& input) {
		// 1. считали данные
		std::vector<std::string> all_queries = std::move(input_reader::ReadInputData(input));
		// 2. разделили информацию об остановке на основную и второстепенную
		auto [info_main, info_secondary] = SplitQueryStopOnMainAndSecondary(all_queries);
		// 3. внесли остановки в справочник
		for (const Stop& stop : info_main) {
			catalogue.AddStop(stop);
		}
		// 4. обрабатываем расстояния и вносим их в справочник
		for (auto [stop_from, line] : info_secondary) {
			std::unordered_map<std::string, size_t> neighbour_stops = ParseLine(line);
			for (const auto& [stop_to, distance] : neighbour_stops) {
				catalogue.SetDistance(stop_from, stop_to, distance);
			}
		}
		// 5. рассортировали данные на маршруты
		std::vector<Bus> buses = std::move(CutBuses(catalogue, all_queries));
		// 6. внесли маршруты в справочник
		for (const Bus& bus : buses) {
			catalogue.AddBus(bus);
		}
	}

	std::pair<InfoMain, InfoSecondary> SplitQueryStopOnMainAndSecondary(
		const std::vector<std::string>& all_queries) {
		std::pair<InfoMain, InfoSecondary> info;
		info.first.reserve(all_queries.size());
		Stop stop;

		for (const std::string& query : all_queries) {
			if (query[0] == 'S') {
				std::string query_stop = query.substr(4); // remove "stop"
				// выделили имя
				auto [name, data] = std::move(Split(query_stop, ':'));
				stop.name = std::string(name.substr(name.find_first_not_of(' ')));
				// выделили широту
				auto [latitude_str, data1] = std::move(Split(data, ','));
				stop.latitude = std::stod(std::string(latitude_str));
				// выделили долготу и расстояния до соседних остановок
				auto [longitude_str, data2] = std::move(Split(data1, ','));
				stop.longitude = std::stod(std::string(longitude_str));

				info.first.push_back(stop);
				if (!data2.empty()) {
					info.second[stop.name] = std::move(std::string(data2));
				}
			}
		}
		return info;
	}

	std::pair<std::string_view, std::string_view> Split(std::string_view line, char symbol) {
		size_t pos = line.find(symbol);
		std::string_view left = line.substr(0, pos);
		left = left.substr(0, left.find_last_not_of(' ') + 1);

		if (pos < line.size() && pos + 1 < line.size()) {
			size_t begin = line.find_first_not_of(' ', pos + 1);
			size_t end = line.find_last_not_of(' ') + 1;
			std::string_view right = line.substr(begin, end - begin);
			return { left, right };
		}
		else {
			return { left, std::string_view() };
		}
	}

	Bus ParseRoute(const TransportCatalogue& catalogue, std::string_view route, char symbol) {
		auto [name, stops] = std::move(Split(route, ':'));

		// выделяем название маршрута
		name = name.substr(name.find_first_not_of(' '));
		// выделяем каждую остановку из общей кучи
		std::deque<const Stop*> stops_ptr = SplitStopsInRoute(catalogue, stops, symbol);

		if (symbol == '-') {
			std::deque<const Stop*> copy = stops_ptr;
			std::move(std::next(copy.rbegin()), copy.rend(), std::back_inserter(stops_ptr));
		}

		TypeRoute type = (symbol == '>') ? TypeRoute::CIRCLE : TypeRoute::DIRECT;

		return { type, std::string(name), stops_ptr };
	}

	std::deque<const Stop*> SplitStopsInRoute(const TransportCatalogue& catalogue,
		std::string_view& stops, char symbol) {
		std::deque<const Stop*> stops_ptr;
		const Stop* stop_ptr = nullptr;
		std::string_view line_stops = stops;

		size_t number = std::count(stops.begin(), stops.end(), symbol);

		for (size_t count = 1; count <= number; ++count) {
			auto [name, line] = Split(line_stops, symbol);

			stop_ptr = catalogue.FindStop(name);
			stops_ptr.push_back(stop_ptr);

			line_stops = line;
		}
		stop_ptr = catalogue.FindStop(line_stops);
		stops_ptr.push_back(stop_ptr);

		return stops_ptr;
	}

	std::unordered_map<std::string, size_t> ParseLine(std::string_view line) {
		if (line.empty()) {
			return {};
		}
		std::unordered_map<std::string, size_t> info;
		std::pair<std::string_view, std::string_view> str;
		str.second = line;

		while (str.second.find(',') != std::string_view::npos) {
			str = Split(str.second, ',');
			auto [distance, stop] = Split(str.first, 'm');
			info[std::string(stop.substr(3))] = std::stoi(std::string(distance));
		}
		auto [distance, stop] = Split(str.second, 'm');
		info[std::string(stop.substr(3))] = std::stoi(std::string(distance));

		return info;
	}

	std::vector<Bus> CutBuses(const TransportCatalogue& catalogue,
		const std::vector<std::string>& all_queries) {
		std::vector<Bus> buses;
		buses.reserve(all_queries.size() / 2);

		for (const std::string& query : all_queries) {

			if (query[0] == 'B') {
				// выделяем нужную информацию об остановке
				buses.push_back(std::move(GenerateInfoAboutBus(catalogue, query)));
			}

		}

		return buses;
	}

	Bus GenerateInfoAboutBus(const TransportCatalogue& catalogue,
		const std::string& query_bus) {

		std::string route = query_bus.substr(3); // remove "Bus"

		if (route.find('-') != std::string::npos) {
			// вызываем метод обработки запроса для прямого маршрута
			return ParseRoute(catalogue, route, '-');
		}
		// вызываем метод обработки запроса для кольцевого маршрута
		return ParseRoute(catalogue, route, '>');
	}

}
