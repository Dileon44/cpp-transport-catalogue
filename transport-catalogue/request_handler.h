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

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusInfo> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через
        const std::unordered_set<const Bus*> GetBusesByStop(const std::string_view& stop_name) const;

        // Этот метод будет нужен в следующей части итогового проекта
        //svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const TransportCatalogue& db_;
        //const renderer::MapRenderer& renderer_;
    };

} // namespace catalogueS