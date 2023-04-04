#pragma once

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <optional>
#include <utility>
#include <vector>
#include <map>

namespace map_renderer {
    inline const double EPSILON = 1e-6;

    struct MapRenderSettings {
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
        template<typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
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
            } else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(Coordinates coords) const {
            return {
                    (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }

        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class MapRenderer {
    public:
        explicit MapRenderer(MapRenderSettings settings) : settings_(settings) {
        }
        void LoadBuses(const std::unordered_map<std::string_view, transport_catalogue::bus::Bus*>& buses);

        void LoadCoordinates(transport_catalogue::TransportCatalogue& catalogue);

        void Render(std::ostream& out);

        std::string Output();

        MapRenderSettings& GetRenderSettings() const;


    private:

        void DrawRoutes(const SphereProjector& proj);

        void DrawRouteName(const SphereProjector& proj);

        void LoadSubstrateForRoutes(std::string_view name, const svg::Point& point);

        void LoadTitleForRoutes(std::string_view name, const svg::Point& point, svg::Color color);

        void DrawStops(const SphereProjector& proj);

        void DrawStopName(const SphereProjector& proj);


        void LoadSubstrateForNameStop(std::string_view name, const svg::Point& point);

        void LoadTitleForNameStop(std::string_view name, const svg::Point& point, svg::Color color);

        MapRenderSettings& settings_;
        svg::Document render_;
        std::vector<Coordinates> coordinates_;
        std::unordered_map<std::string_view, transport_catalogue::bus::Bus*> buses_;
        std::set<std::string_view> bus_names_;
        std::map<std::string_view, transport_catalogue::stop::Stop*> correct_stops_;
    };
}