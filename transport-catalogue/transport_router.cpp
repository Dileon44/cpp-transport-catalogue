#include "transport_router.h"
#include "router.h"

namespace catalogue {
	std::optional<graph::Router<double>::RouteInfo> TransportRouter::BuildRoute(
		VertexId from, VertexId to) const {
		return router_ptr_->BuildRoute(from, to);
	}
}