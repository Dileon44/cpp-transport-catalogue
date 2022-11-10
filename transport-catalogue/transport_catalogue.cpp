#include "transport_catalogue.h"
#include "geo.h"

#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <unordered_map>
#include <string_view>


namespace catalogue {

	using InfoMain = std::vector<Stop>;
	using InfoSecondary = std::unordered_map<std::string, std::string>;

	void TransportCatalogue::AddStop(const Stop& stop) {
		stops_.push_back(stop);
		const Stop* stop_ptr = &stops_[stops_.size() - 1];
		stop_to_ptr_stop_.insert({ stop_ptr->name, stop_ptr });

		if (!stop_to_buses_.count(FindStop(stop.name))) {
			stop_to_buses_[stop_ptr];
		}
	}

	void TransportCatalogue::AddBus(const Bus& bus) {
		buses_.push_back(bus);
		const Bus* bus_ptr = &buses_[buses_.size() - 1];
		bus_to_ptr_bus_.insert({ bus_ptr->name, bus_ptr });

		for (const Stop* stop_ptr : bus.stops) {
			if (stop_to_buses_.count(stop_ptr)) {
				stop_to_buses_.at(stop_ptr).insert(bus_ptr);
			}
			else {
				stop_to_buses_[stop_ptr].insert(bus_ptr);
			}
		}
	}

	const Bus* TransportCatalogue::FindBus(std::string_view bus) const {
		return (bus_to_ptr_bus_.count(bus)) ? bus_to_ptr_bus_.at(bus) : nullptr;
	}

	const Stop* TransportCatalogue::FindStop(std::string_view stop) const {
		return (stop_to_ptr_stop_.count(stop)) ? stop_to_ptr_stop_.at(stop) : nullptr;
	}

	void TransportCatalogue::SetDistance(std::string_view from, std::string_view to, size_t distance) {

		if (!distances_.count({ FindStop(from), FindStop(to) })) {
			distances_.insert({ {FindStop(from), FindStop(to)}, distance });
		}
	}

	BusInfo TransportCatalogue::GetBusInfo(std::string_view bus) const {

		BusInfo bus_info;

		if (bus_to_ptr_bus_.count(bus)) {
			bus_info.bus_is_existing = true;
			bus_info.name = FindBus(bus)->name;
			bus_info.number_stops = bus_to_ptr_bus_.at(bus)->stops.size();
			bus_info.unique_stops = CalculateUniqueStops(bus);
			bus_info.length_route = CalculateFactLenghtRoute(bus);
			bus_info.curvature = bus_info.length_route / CalculateGeoLenghtRoute(bus);
		}
		else {
			bus_info.name = std::string(bus);
		}

		return bus_info;
	}

	StopInfo TransportCatalogue::GetStopInfo(std::string_view stop) const {
		if (stop_to_buses_.count(FindStop(stop))) {
			return { true, std::string(stop), stop_to_buses_.at(FindStop(stop)) };
		}

		return { false, std::string(stop), {} };
	}

	size_t TransportCatalogue::GetDistance(std::string_view from, std::string_view to) const {

		if (distances_.count({ FindStop(from), FindStop(to) })) {
			return distances_.at({ FindStop(from), FindStop(to) });
		}

		return distances_.at({ FindStop(to), FindStop(from) });
	}

	class HasherUnique {
	public:
		size_t operator()(const Stop* stop) const {
			return hasher_(stop);
		}

	private:
		std::hash<const Stop*> hasher_;
	};

	size_t TransportCatalogue::CalculateUniqueStops(std::string_view bus) const {
		std::unordered_set<const Stop*, HasherUnique> tmp;

		for (const Stop* stop : FindBus(bus)->stops) {
			if (!tmp.count(stop)) {
				tmp.insert(stop);
			}
		}

		return tmp.size();
	}

	double TransportCatalogue::CalculateFactLenghtRoute(std::string_view bus) const {
		double distance = 0;

		auto from = FindBus(bus)->stops.begin();
		for (auto to = std::next(from); to != FindBus(bus)->stops.end(); ++to) {
			distance += GetDistance((*from)->name, (*to)->name);
			from = to;
		}

		return distance;
	}

	double TransportCatalogue::CalculateGeoLenghtRoute(std::string_view bus) const {
		double distance = 0;

		geo::Coordinates from = { FindBus(bus)->stops[0]->coordinate.lat, FindBus(bus)->stops[0]->coordinate.lng };

		for (size_t n = 1; n < FindBus(bus)->stops.size(); ++n) {
			auto stop = FindBus(bus)->stops[n];
			geo::Coordinates to{ stop->coordinate.lat, stop->coordinate.lng };
			distance += ComputeDistance(from, to);
			from = to;
		}
		return distance;
	}

	const std::unordered_set<const Bus*> TransportCatalogue::GetAllBuses() const {
		std::unordered_set<const Bus*> buses;
		for (const auto& [_, bus_ptr] : bus_to_ptr_bus_) {
			buses.insert(bus_ptr);
		}
		return buses;
	}

	const std::unordered_set<const Stop*> TransportCatalogue::GetAllStops() const {
		std::unordered_set<const Stop*> stops;
		for (const auto& [_, stop_ptr] : stop_to_ptr_stop_) {
			stops.insert(stop_ptr);
		}
		return stops;
	}

} // namespace catalogue
