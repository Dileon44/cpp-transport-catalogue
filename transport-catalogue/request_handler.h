#pragma once

#include <optional>
#include <string_view>
#include <unordered_set>

#include "domain.h"
#include "transport_catalogue.h"

namespace catalogue {

    class RequestHandler {
    public:
        RequestHandler(const TransportCatalogue& db);

        std::optional<BusInfo> GetBusStat(const std::string_view& bus_name) const;

        const std::unordered_set<const Bus*> GetBusesByStop(const std::string_view& stop_name) const;

        //svg::Document RenderMap() const;

    private:
        const TransportCatalogue& db_;
        //const renderer::MapRenderer& renderer_;
    };

} // namespace catalogueS
