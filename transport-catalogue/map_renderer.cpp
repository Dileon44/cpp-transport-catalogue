#include "map_renderer.h"

#include <map>
#include <string_view>
#include <algorithm>

namespace catalogue {

    namespace renderer {

        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

        svg::Document MapRenderer::RenderMap(const renderer::RenderSettings& render_settings,
                                             const std::set< const Bus*, CompareBuses >& buses) const {
            // 1. upload all routes and coordinates of stops 
            //    that are included in these routes
            auto [map_buses_to_geo_coords_of_stops, all_geo_coords] =
                HighlightBusesAndCoordinatesOfStops(buses);

            // 2. include projector on a plane
            const SphereProjector proj = {
                all_geo_coords.begin(), all_geo_coords.end(),
                render_settings.width,
                render_settings.height,
                render_settings.padding
            };
            // 3.
            svg::Document document = std::move(
                CreateVisualization(render_settings, buses, map_buses_to_geo_coords_of_stops, proj)
            );
            return document;
        }

        std::pair<BusesCoordinates, std::vector<geo::Coordinates>> 
            MapRenderer::HighlightBusesAndCoordinatesOfStops(
                const std::set< const Bus*, CompareBuses >& buses) const {

            BusesCoordinates bus_geo_coords; // BusesCoordinates defind in MapRenderer.h
            std::vector<geo::Coordinates> all_geo_coords;
            for (const Bus* bus : buses) {
                std::vector<geo::Coordinates> geo_coords;
                for (const auto& stop : bus->stops) {
                    geo_coords.push_back({ stop->coordinate.lat, stop->coordinate.lng });
                    all_geo_coords.push_back({ stop->coordinate.lat, stop->coordinate.lng });
                }
                bus_geo_coords.insert({ bus, std::move(geo_coords) });
            }

            return std::pair{ bus_geo_coords, all_geo_coords };
        }

        svg::Document MapRenderer::CreateVisualization(const renderer::RenderSettings& render_settings, 
                                                       const std::set< const Bus*, CompareBuses >& buses,
                                                       const BusesCoordinates& bus_geo_coords,
                                                       const SphereProjector proj) const {
            // 1.
            svg::Document document = CreatePolylinesRoutes(render_settings, bus_geo_coords, proj);
            // 2.
            CreateRouteNames(render_settings, buses, document, proj);

            // collecting all stops
            std::set<const Stop*, CompareStop> stops;
            for (const Bus* bus : buses) {
                for (const Stop* stop : bus->stops) {
                    stops.insert(stop);
                }
            }
            // 3.
            CreateStopIcons(render_settings, document, stops, proj);
            // 4.
            CreateStopNames(render_settings, document, stops, proj);

            return document;
        }

        svg::Document MapRenderer::CreatePolylinesRoutes(const renderer::RenderSettings& render_settings, 
                                                         const BusesCoordinates&  bus_geo_coords,
                                                         const SphereProjector proj) const {
            svg::Document document;

            size_t count = 0;
            for (const auto& [bus, geo_coords] : bus_geo_coords) {
                if (bus->stops.empty()) {
                    continue;
                }
                svg::Polyline polyline;
                for (const auto& geo_coord : geo_coords) {
                    polyline.AddPoint(proj(geo_coord));
                }
                SetPathPropsAttributesPolyline(render_settings, polyline, count);
                document.Add(polyline);
                
                ++count;
            }
            return document;
        }

        void MapRenderer::SetPathPropsAttributesPolyline(const renderer::RenderSettings& render_settings, 
                                                         svg::Polyline& polyline,
                                                         size_t count) const {
            polyline.SetFillColor(svg::NoneColor)
                    .SetStrokeColor(render_settings.color_palette
                        .at(count % render_settings.color_palette.size()))
                    .SetStrokeWidth(render_settings.line_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        }

        void MapRenderer::CreateRouteNames(const renderer::RenderSettings& render_settings
                                          , const std::set< const Bus*, CompareBuses >& buses
                                          , svg::Document& document, const SphereProjector proj) const {
            size_t count = 0;
            for (const Bus* bus : buses) {
                if (bus->stops.empty()) {
                    continue;
                }
                geo::Coordinates route_begin = { bus->stops[0]->coordinate.lat, 
                                                 bus->stops[0]->coordinate.lng };
                geo::Coordinates route_end = { bus->stops[(bus->stops.size())/2]->coordinate.lat,
                                               bus->stops[(bus->stops.size())/2]->coordinate.lng };
                auto [background_begin, title_begin] = std::move(
                    CreateBackgroundAndTitleForRoute(
                        render_settings, proj(route_begin), bus->name, count)
                );
                document.Add(background_begin);
                document.Add(title_begin);
                if (bus->type_route == TypeRoute::DIRECT && route_begin != route_end) {
                    auto [background_end, title_end] = std::move(
                        CreateBackgroundAndTitleForRoute(
                            render_settings, proj(route_end), bus->name, count)
                    );
                    document.Add(background_end);
                    document.Add(title_end);
                }
                ++count;
            }
        }

        std::pair<svg::Text, svg::Text> MapRenderer::CreateBackgroundAndTitleForRoute(
            const renderer::RenderSettings& render_settings,
            svg::Point point, std::string_view bus_name, size_t count) const {

            svg::Text background;
            svg::Text title;

            background.SetPosition(point)
                .SetOffset({ render_settings.bus_label_offset[0],
                             render_settings.bus_label_offset[1] })
                .SetFontSize(render_settings.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(std::string(bus_name))
                .SetFillColor(render_settings.underlayer_color)
                .SetStrokeColor(render_settings.underlayer_color)
                .SetStrokeWidth(render_settings.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            title.SetPosition(point)
                .SetOffset({ render_settings.bus_label_offset[0],
                             render_settings.bus_label_offset[1] })
                .SetFontSize(render_settings.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(std::string(bus_name))
                .SetFillColor(render_settings.color_palette
                    .at(count % render_settings.color_palette.size()));

            return {background, title};
        }

        void MapRenderer::CreateStopIcons(const renderer::RenderSettings& render_settings 
                                         ,  svg::Document& document
                                         , const std::set<const Stop*, CompareStop>& stops
                                         , const SphereProjector proj) const {
            svg::Circle circle;
            // creating default settings for all stops
            circle.SetRadius(render_settings.stop_radius).SetFillColor("white");
            // create stop icons
            for (const Stop* stop : stops) {
                circle.SetCenter(proj({ stop->coordinate.lat, stop->coordinate.lng }));
                document.Add(circle);
            }
        }

        void MapRenderer::CreateStopNames(const renderer::RenderSettings& render_settings
                                         , svg::Document& document
                                         , const std::set<const Stop*, CompareStop>& stops
                                         , const SphereProjector proj) const {
            svg::Text background;
            svg::Text title;
            for (const Stop* stop : stops) {
                background.SetPosition(proj({ stop->coordinate.lat, stop->coordinate.lng }))
                    .SetOffset({ render_settings.stop_label_offset[0],
                                 render_settings.stop_label_offset[1] })
                    .SetFontSize(render_settings.stop_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetData(stop->name)
                    .SetFillColor(render_settings.underlayer_color)
                    .SetStrokeColor(render_settings.underlayer_color)
                    .SetStrokeWidth(render_settings.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                title.SetPosition(proj({ stop->coordinate.lat, stop->coordinate.lng }))
                    .SetOffset({ render_settings.stop_label_offset[0],
                                 render_settings.stop_label_offset[1] })
                    .SetFontSize(render_settings.stop_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetData(stop->name)
                    .SetFillColor("black");
                document.Add(background);
                document.Add(title);
            }
        }

    } // namespace renderer

} // namespace catalogue
