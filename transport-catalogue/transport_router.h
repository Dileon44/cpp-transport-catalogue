#pragma once

#include "router.h"

#include <memory>

namespace catalogue {

	class TransportRouter {
		using VertexId = size_t;
		using Graph = graph::DirectedWeightedGraph<double>;
	public:
		TransportRouter(const Graph& graph) {
			router_ptr_ = std::make_unique<graph::Router<double>>(graph::Router(graph));
		}

		std::optional<graph::Router<double>::RouteInfo> BuildRoute(VertexId from, VertexId to) const;

	private:
		std::unique_ptr<graph::Router<double>> router_ptr_;
	};

} // namespace catalogue