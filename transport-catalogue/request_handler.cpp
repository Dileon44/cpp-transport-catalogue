#include "request_handler.h"

#include <string_view>

namespace catalogue {

	RequestHandler::RequestHandler(const catalogue::TransportCatalogue& db) : db_(db) {}

	std::optional<BusInfo> RequestHandler::GetBusStat(
		const std::string_view& bus_name) const {

		return db_.GetBusInfo(bus_name);
	}

	const std::unordered_set<const Bus*> RequestHandler::GetBusesByStop(
		const std::string_view& stop_name) const {

		const std::unordered_set<const Bus*> a = db_.GetStopInfo(stop_name).buses;

		return a;
	}

} // namespace catalogue