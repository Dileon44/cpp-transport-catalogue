#include "transport_router.h"
#include "router.h"

namespace catalogue {

	void TransportRouter::BuildGraphAndRouter() {
		BuildVertexes();

		size_t number_all_stops = stop_to_pair_vertex_.size();
		graph_ = std::make_unique<Graph>(Graph(2 * number_all_stops));
		edges_info_.reserve(2 * number_all_stops);

		for (const auto& [stop, pair_vertexes] : stop_to_pair_vertex_) {
			graph_->AddEdge(graph::Edge<double>{
				pair_vertexes.begin_wait, pair_vertexes.end_wait, routing_settings_.bus_wait_time
			});
			edges_info_.push_back(EdgeWaitInfo{ stop, routing_settings_.bus_wait_time });
		}

		for (const Bus* bus : catalogue_.GetAllBuses()) {
			if (bus->type_route == TypeRoute::CIRCLE) {
				AddEdgeBusInfo(bus->stops.begin(), bus->stops.end(), bus);
			}
			else {
				size_t half = (bus->stops.size() + 1) / 2;
				auto middle_it = std::next(bus->stops.begin(), half);
				auto rmiddle_it = std::next(bus->stops.rbegin(), half);
				AddEdgeBusInfo(bus->stops.begin(), middle_it, bus);
				AddEdgeBusInfo(rmiddle_it - 1, bus->stops.rend(), bus);
			}
		}
		router_ = std::make_unique<Router>(Router(*graph_));
	}

	void TransportRouter::BuildVertexes() {
		size_t count_vertex = 0;
		for (const Stop* stop : catalogue_.GetAllStops()) {
			stop_to_pair_vertex_.insert({ stop, PairVertexesId{ count_vertex++, count_vertex++ } });
		}
	}

	std::optional<graph::Router<double>::RouteInfo> TransportRouter::BuildRoute(
		VertexId from, VertexId to) const {
		return router_->BuildRoute(from, to);
	}

	const PairVertexesId& TransportRouter::GetPairVertexesId(std::string stop_name) const {
		return stop_to_pair_vertex_.at(catalogue_.FindStop(stop_name));
	}

	const EdgeInfo& TransportRouter::GetEdgeInfo(const EdgeId edge_id) const {
		return edges_info_.at(edge_id);
	}
}
