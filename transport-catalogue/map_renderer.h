#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <vector>
#include <set>
#include <map>
#include <string_view>

namespace catalogue {

    namespace renderer {

        inline const double EPSILON = 1e-6;
        bool IsZero(double value);

        struct RenderSettings {
            double width;
            double height;
            double padding;
            double line_width;
            double stop_radius;

            int bus_label_font_size;
            std::vector<double> bus_label_offset;

            int stop_label_font_size;
            std::vector<double> stop_label_offset;

            svg::Color underlayer_color;
            double underlayer_width;

            std::vector<svg::Color> color_palette;
        };


        class SphereProjector {
        public:
            // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
            template <typename PointInputIt>
            SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                double max_width, double max_height, double padding);

            // Проецирует широту и долготу в координаты внутри SVG-изображения
            svg::Point operator()(geo::Coordinates coords) const;

        private:
            double padding_;
            double min_lon_ = 0;
            double max_lat_ = 0;
            double zoom_coeff_ = 0;
        };

        using BusesCoordinates = std::map<const Bus*, std::vector<geo::Coordinates>, CompareBuses>;

        class MapRenderer {
        public:

            RenderSettings& GetRenderSettings();
            void AddBus(const Bus* bus);

            svg::Document RenderMap() const;

        private:
            RenderSettings render_settings_;
            std::set< const Bus*, CompareBuses > buses_; // CompareBuses defined in domain.h

            std::pair<BusesCoordinates, std::vector<geo::Coordinates>>
                HighlightBusesAndCoordinatesOfStops() const;

            svg::Document CreateVisualization(const BusesCoordinates& bus_geo_coords,
                const SphereProjector proj) const;

            svg::Document CreatePolylinesRoutes(const BusesCoordinates& bus_geo_coords,
                                                const SphereProjector proj) const;

            //template<typename Obj>
            //void SetPathPropsAttributesPolyline(svg::PathProps<Obj>* polyline, size_t count);

            void SetPathPropsAttributesPolyline(svg::Polyline& polyline, size_t count) const;

            void CreateRouteNames(svg::Document& document, const SphereProjector proj) const;

            std::pair<svg::Text, svg::Text> CreateBackgroundAndTitleForRoute(
                svg::Point point, std::string_view bus_name, size_t count) const;

            void CreateStopIcons(svg::Document& document
                                , const std::set<const Stop*, CompareStop>& stops
                                , const SphereProjector proj) const;

            void CreateStopNames(svg::Document& document
                                , const std::set<const Stop*, CompareStop>& stops
                                , const SphereProjector proj) const;
        };

        template <typename PointInputIt>
        SphereProjector::SphereProjector(
            PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        //template<typename Obj>
        //void MapRenderer::SetPathPropsAttributesPolyline(svg::PathProps<Obj>* object, size_t count) {
        //    object->SetFillColor(svg::NoneColor)
        //        .SetStrokeColor(render_settings_.color_palette
        //            .at(count % render_settings_.color_palette.size()))
        //        .SetStrokeWidth(render_settings_.line_width)
        //        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        //        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        //}

    } // namespace renderer

} // namespace catalogue