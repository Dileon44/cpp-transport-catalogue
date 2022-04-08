#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <set>
#include <functional>

namespace catalogue {

	enum class TypeRoute {
		CIRCLE,
		DIRECT
	};

	struct Stop {
		std::string name;
		double latitude;
		double longitude;
	};

	struct Bus {
		TypeRoute type_route;
		std::string name;
		std::deque<const Stop*> stops;
	};

	struct BusInfo {
		bool bus_is_existing = false;
		std::string name;
		size_t number_stops = 0;
		size_t unique_stops = 0;
		double length_route = 0;
		double curvature = 0;
	};

	struct StopInfo {
		bool to_exist = false;
		std::string name;
		std::set<std::string_view> buses;
	};

	class TransportCatalogue {

		using InfoMain = std::vector<Stop>;
		using InfoSecondary = std::unordered_map<std::string, std::string>;

	public:
		void AddStop(const Stop& stop);
		void AddBus(const Bus& bus);
		void SetDistance(std::string_view from, std::string_view to, size_t distance);

		const Bus* FindBus(std::string_view bus) const;
		const Stop* FindStop(std::string_view stop) const;

		BusInfo GetBusInfo(std::string_view bus) const;
		StopInfo GetStopInfo(std::string_view stop) const;
		size_t GetDistance(std::string_view from, std::string_view to) const;

	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, const Stop*> stop_to_ptr_stop_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, const Bus*> bus_to_ptr_bus_;
		std::unordered_map<const Stop*, std::set<std::string_view>> stop_to_buses_;

		class HasherDistance {
		public:
			size_t operator()(std::pair<const Stop*, const Stop*> key) const {
				return hasher_(key.first) + hasher_(key.second) * 16;
			}

		private:
			std::hash<const Stop*> hasher_;
		};

		std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, HasherDistance> distances_;

		size_t CalculateUniqueStops(std::string_view bus) const;

		double CalculateFactLenghtRoute(std::string_view bus) const;

		double CalculateGeoLenghtRoute(std::string_view bus) const;
	};

}
