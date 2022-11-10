#pragma once

#include <optional>
#include <string_view>
#include <unordered_set>
#include <deque>

#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace catalogue {

    class RequestHandler {
    public:
        RequestHandler(const TransportCatalogue& db
                      , const renderer::MapRenderer& renderer
                      , const TransportRouter& router);

        std::optional<BusInfo> GetBusStat(const std::string_view& bus_name) const;

        const std::unordered_set<const Bus*> GetBusesByStop(const std::string_view& stop_name) const;

        const std::deque<const Stop*>& GetStopsByBus(const std::string_view bus_name) const;

        const std::unordered_set<const Bus*> GetAllBuses() const;

        const std::unordered_set<const Stop*> GetAllStops() const;

        svg::Document RenderMap(const renderer::RenderSettings& render_settings,
            const std::set< const Bus*, CompareBuses >& buses) const;

        std::optional<graph::Router<double>::RouteInfo> BuildRoute(
            std::string_view stop_from, std::string_view stop_to) const;

    private:
        const TransportCatalogue& db_;
        const renderer::MapRenderer& renderer_;
        const TransportRouter& router_;
    };

} // namespace catalogueS
