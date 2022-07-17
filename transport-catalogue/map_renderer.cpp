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

        RenderSettings& MapRenderer::GetRenderSettings() {
            return render_settings_;
        }

        void MapRenderer::AddBus(const Bus* bus) {
            buses_.insert(bus);
        }

        svg::Document MapRenderer::RenderMap() const {
            // 1. upload all routes and coordinates of stops 
            //    that are included in these routes
            auto [map_buses_to_geo_coords_of_stops, all_geo_coords] =
                HighlightBusesAndCoordinatesOfStops();

            // 2. include projector on a plane
            const SphereProjector proj = {
                all_geo_coords.begin(), all_geo_coords.end(),
                render_settings_.width, 
                render_settings_.height,
                render_settings_.padding
            };
            // 3.
            svg::Document document = std::move(
                CreateVisualization(map_buses_to_geo_coords_of_stops, proj)
            );
            return document;
        }

        std::pair<BusesCoordinates, std::vector<geo::Coordinates>> 
            MapRenderer::HighlightBusesAndCoordinatesOfStops() const {

            BusesCoordinates bus_geo_coords; // BusesCoordinates defind in MapRenderer.h
            std::vector<geo::Coordinates> all_geo_coords;
            for (const Bus* bus : buses_) {
                std::vector<geo::Coordinates> geo_coords;
                for (const auto& stop : bus->stops) {
                    geo_coords.push_back({ stop->latitude, stop->longitude });
                    all_geo_coords.push_back({ stop->latitude, stop->longitude });
                }
                bus_geo_coords.insert({ bus, std::move(geo_coords) });
            }

            return std::pair{ bus_geo_coords, all_geo_coords };
        }

        svg::Document MapRenderer::CreateVisualization(const BusesCoordinates& bus_geo_coords,
                                                       const SphereProjector proj) const {
            // 1.
            svg::Document document = CreatePolylinesRoutes(bus_geo_coords, proj);
            // 2.
            CreateRouteNames(document, proj);

            // collecting all stops
            std::set<const Stop*, CompareStop> stops;
            for (const Bus* bus : buses_) {
                for (const Stop* stop : bus->stops) {
                    stops.insert(stop);
                }
            }
            // 3. 
            CreateStopIcons(document, stops, proj);
            // 4.
            CreateStopNames(document, stops, proj);

            return document;
        }

        svg::Document MapRenderer::CreatePolylinesRoutes(const BusesCoordinates&  bus_geo_coords,
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
                SetPathPropsAttributesPolyline(polyline, count);
                document.Add(polyline);

                ++count;
            }

            return document;
        }

        void MapRenderer::SetPathPropsAttributesPolyline(svg::Polyline& polyline,
                                                 size_t count) const {
            polyline.SetFillColor(svg::NoneColor)
                    .SetStrokeColor(render_settings_.color_palette
                        .at(count % render_settings_.color_palette.size()))
                    .SetStrokeWidth(render_settings_.line_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        }

        void MapRenderer::CreateRouteNames(svg::Document& document, const SphereProjector proj) const {
            size_t count = 0;
            for (const Bus* bus : buses_) {
                if (bus->stops.empty()) {
                    continue;
                }
                geo::Coordinates route_begin = { bus->stops[0]->latitude, 
                                                 bus->stops[0]->longitude };
                geo::Coordinates route_end = { bus->stops[(bus->stops.size())/2]->latitude,
                                               bus->stops[(bus->stops.size())/2]->longitude };
                auto [background_begin, title_begin] = std::move(
                    CreateBackgroundAndTitleForRoute(proj(route_begin), bus->name, count));
                document.Add(background_begin);
                document.Add(title_begin);
                if (bus->type_route == TypeRoute::DIRECT && route_begin != route_end) {
                    auto [background_end, title_end] = std::move(
                        CreateBackgroundAndTitleForRoute(proj(route_end), bus->name, count));
                    document.Add(background_end);
                    document.Add(title_end);
                }
                ++count;
            }
        }

        std::pair<svg::Text, svg::Text> MapRenderer::CreateBackgroundAndTitleForRoute(
            svg::Point point, std::string_view bus_name, size_t count) const {

            svg::Text background;
            svg::Text title;

            background.SetPosition(point)
                .SetOffset({ render_settings_.bus_label_offset[0],
                             render_settings_.bus_label_offset[1] })
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(std::string(bus_name))
                .SetFillColor(render_settings_.underlayer_color)
                .SetStrokeColor(render_settings_.underlayer_color)
                .SetStrokeWidth(render_settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            title.SetPosition(point)
                .SetOffset({ render_settings_.bus_label_offset[0],
                             render_settings_.bus_label_offset[1] })
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(std::string(bus_name))
                .SetFillColor(render_settings_.color_palette
                    .at(count % render_settings_.color_palette.size()));

            return {background, title};
        }

        void MapRenderer::CreateStopIcons(svg::Document& document
                                         , const std::set<const Stop*, CompareStop>& stops
                                         , const SphereProjector proj) const {
            svg::Circle circle;
            // creating default settings for all stops
            circle.SetRadius(render_settings_.stop_radius).SetFillColor("white");
            // create stop icons
            for (const Stop* stop : stops) {
                circle.SetCenter(proj({ stop->latitude, stop->longitude }));
                document.Add(circle);
            }
        }

        void MapRenderer::CreateStopNames(svg::Document& document
                                         , const std::set<const Stop*, CompareStop>& stops
                                         , const SphereProjector proj) const {
            svg::Text background;
            svg::Text title;
            for (const Stop* stop : stops) {
                background.SetPosition(proj({ stop->latitude, stop->longitude }))
                    .SetOffset({ render_settings_.stop_label_offset[0],
                                 render_settings_.stop_label_offset[1] })
                    .SetFontSize(render_settings_.stop_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetData(stop->name)
                    .SetFillColor(render_settings_.underlayer_color)
                    .SetStrokeColor(render_settings_.underlayer_color)
                    .SetStrokeWidth(render_settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                title.SetPosition(proj({ stop->latitude, stop->longitude }))
                    .SetOffset({ render_settings_.stop_label_offset[0],
                                 render_settings_.stop_label_offset[1] })
                    .SetFontSize(render_settings_.stop_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetData(stop->name)
                    .SetFillColor("black");
                document.Add(background);
                document.Add(title);
            }
        }

    } // namespace renderer

}