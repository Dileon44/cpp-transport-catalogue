#include "request_handler.h"

#include <string_view>
#include <deque>

namespace catalogue {

	RequestHandler::RequestHandler(const catalogue::TransportCatalogue& db,
		const renderer::MapRenderer& renderer, const TransportRouter& router)
		: db_(db), renderer_(renderer), router_(router) {
	}

	std::optional<BusInfo> RequestHandler::GetBusStat(
		const std::string_view& bus_name) const {

		return db_.GetBusInfo(bus_name);
	}

	const std::unordered_set<const Bus*> RequestHandler::GetBusesByStop(
		const std::string_view& stop_name) const {

		const std::unordered_set<const Bus*> a = db_.GetStopInfo(stop_name).buses;

		return a;
	}

	const std::deque<const Stop*>& RequestHandler::GetStopsByBus(const std::string_view bus_name) const {
		return db_.FindBus(bus_name)->stops;
	}

	const std::unordered_set<const Bus*> RequestHandler::GetAllBuses() const {
		return db_.GetAllBuses();
	}

	const std::unordered_set<const Stop*> RequestHandler::GetAllStops() const {
		return db_.GetAllStops();
	}

	svg::Document RequestHandler::RenderMap(const renderer::RenderSettings& render_settings,
		const std::set< const Bus*, CompareBuses >& buses) const {
		return renderer_.RenderMap(render_settings, buses);
	}

	std::optional<graph::Router<double>::RouteInfo> RequestHandler::BuildRoute(
		std::string_view stop_from, std::string_view stop_to) const {
		return router_.BuildRoute(stop_from, stop_to);
	}

} // namespace catalogue
