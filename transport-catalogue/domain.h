#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <set>
#include <unordered_set>
#include <deque>
#include <optional>
#include <variant>

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

	using EdgeId = size_t;
	using VertexId = size_t;

	struct EdgeWaitInfo {
		const Stop* stop = nullptr;
		double weight = 0;
	};

	struct EdgeBusInfo {
		const Bus* bus = nullptr;
		double weight = 0;
		size_t span_count = 0;
	};

	using EdgeInfo = std::variant<EdgeWaitInfo, EdgeBusInfo>;

	struct RoutingSettings {
		double bus_velocity = 40; // meter per minute
		double bus_wait_time = 6; // minute
	};

	struct PairVertexesId {
		VertexId begin_wait;
		VertexId end_wait;
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
