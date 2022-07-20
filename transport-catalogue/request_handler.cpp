#include "request_handler.h"

#include <string_view>

namespace catalogue {

	RequestHandler::RequestHandler(const catalogue::TransportCatalogue& db,
								   const renderer::MapRenderer& renderer)
		: db_(db), renderer_(renderer) {
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

	svg::Document RequestHandler::RenderMap(const renderer::RenderSettings& render_settings,
											const std::set< const Bus*, CompareBuses >& buses) const {
		return renderer_.RenderMap(render_settings, buses);
	}

} // namespace catalogue
