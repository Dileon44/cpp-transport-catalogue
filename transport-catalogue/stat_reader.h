#pragma once

#include <istream>
#include <string>

#include "transport_catalogue.h"

namespace catalogue {

	namespace stat_reader {

		std::string_view CutName(std::string_view unprocessed_request);

		void PrintInfoBus(const BusInfo& bus);

		void PrintInfoStop(const StopInfo& bus);

		void SearchInfo(const TransportCatalogue& catalogue, std::istream& input);

	}

}
