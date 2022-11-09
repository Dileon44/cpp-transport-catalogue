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
#include "graph.h"

namespace catalogue {
	using Graph = graph::DirectedWeightedGraph<double>;
	using EdgeInfo = std::variant<EdgeWaitInfo, EdgeBusInfo>;

	class TransportCatalogue {

		using InfoMain = std::vector<Stop>;
		using InfoSecondary = std::unordered_map<std::string, std::string>;

	public:
		void AddStop(const Stop& stop);
		void AddBus(const Bus& bus);

		const Bus* FindBus(std::string_view bus) const;
		const Stop* FindStop(std::string_view stop) const;

		void SetRoutingSettings(RoutingSettings routing_settings);
		void SetDistance(std::string_view from, std::string_view to, size_t distance);

		BusInfo GetBusInfo(std::string_view bus) const;
		StopInfo GetStopInfo(std::string_view stop) const;
		size_t GetDistance(std::string_view from, std::string_view to) const;
		const std::unordered_set<const Bus*> GetAllBuses() const;
		const std::unordered_set<const Stop*> GetAllStops() const;
		const Graph& GetGraph() const;
		const PairVertexesId& GetPairVertexesId(std::string stop_name) const;
		const EdgeInfo& GetEdgeInfo(const EdgeId edge_id) const;

		void BuildGraph();

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

		RoutingSettings routing_settings_;
		std::unordered_map<const Stop*, PairVertexesId> stop_to_pair_vertex_;
		std::unique_ptr<Graph> graph_ptr_;
		std::vector<EdgeInfo> edges_info_;

		size_t CalculateUniqueStops(std::string_view bus) const;
		double CalculateFactLenghtRoute(std::string_view bus) const;
		double CalculateGeoLenghtRoute(std::string_view bus) const;

		void BuildVertexes();

		template<typename Iter>
		void AddEdgeBusInfo(Iter first, Iter last, const Bus* bus);
	};

	template<typename Iter>
	void TransportCatalogue::AddEdgeBusInfo(Iter first, Iter last, const Bus* bus) {

		for (auto from = first; from != last; ++from) {
			size_t distance = 0;
			size_t span_count = 0;
			VertexId from_vertex = stop_to_pair_vertex_.at(*from).end_wait;
			for (auto to = std::next(from); to != last; ++to) {
				auto before_to = std::prev(to);
				distance += GetDistance((*before_to)->name, (*to)->name);
				++span_count;

				VertexId to_vertex = stop_to_pair_vertex_.at(*to).begin_wait;
				double weight = double(distance) / routing_settings_.bus_velocity;

				graph_ptr_->AddEdge(graph::Edge<double>{ from_vertex, to_vertex, weight });
				edges_info_.push_back(EdgeBusInfo{ bus, weight, span_count });
			}
		}
	}

} // namespace catalogue
