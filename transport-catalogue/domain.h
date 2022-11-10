#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <set>
#include <unordered_set>
#include <deque>
#include <optional>

namespace catalogue {

	enum class TypeRoute {
		CIRCLE,
		DIRECT
	};

	struct Stop {
		std::string name;
		geo::Coordinates coordinate;
	};

	struct Bus {
		TypeRoute type_route;
		std::string name;
		std::deque<const Stop*> stops;
	};

	struct BusInfo {
		bool bus_is_existing = false;
		std::string name = "unknown bus";
		size_t number_stops = 0;
		size_t unique_stops = 0;
		double length_route = 0;
		double curvature = 0;
	};

	struct StopInfo {
		bool to_exist = false;
		std::string name = "unknown stop";
		std::unordered_set<const Bus*> buses;
	};

	struct CompareBuses {
	public:
		bool operator()(const Bus* l, const Bus* r) const;
	};

	struct CompareStop {
	public:
		bool operator()(const Stop* l, const Stop* r) const;
	};

} // namespace catalogue
