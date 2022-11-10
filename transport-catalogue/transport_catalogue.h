#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <set>
#include <functional>
#include <variant>

#include "domain.h"

namespace catalogue {
	class TransportCatalogue {
		using InfoMain = std::vector<Stop>;
		using InfoSecondary = std::unordered_map<std::string, std::string>;

	public:
		void AddStop(const Stop& stop);
		void AddBus(const Bus& bus);

		const Bus* FindBus(std::string_view bus) const;
		const Stop* FindStop(std::string_view stop) const;

		void SetDistance(std::string_view from, std::string_view to, size_t distance);

		BusInfo GetBusInfo(std::string_view bus) const;
		StopInfo GetStopInfo(std::string_view stop) const;
		size_t GetDistance(std::string_view from, std::string_view to) const;
		const std::unordered_set<const Bus*> GetAllBuses() const;
		const std::unordered_set<const Stop*> GetAllStops() const;

	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, const Stop*> stop_to_ptr_stop_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, const Bus*> bus_to_ptr_bus_;
		std::unordered_map<const Stop*, std::unordered_set<const Bus*>> stop_to_buses_;

		class HasherDistance {
		public:
			size_t operator()(std::pair<const Stop*, const Stop*> key) const {
				return hasher_(key.first) + hasher_(key.second) * 16;
			}
		private:
			std::hash<const Stop*> hasher_;
		};

		std::unordered_map<std::pair<const Stop*, const Stop*>,	size_t, HasherDistance> distances_;

		size_t CalculateUniqueStops(std::string_view bus) const;
		double CalculateFactLenghtRoute(std::string_view bus) const;
		double CalculateGeoLenghtRoute(std::string_view bus) const;
	};

} // namespace catalogue
