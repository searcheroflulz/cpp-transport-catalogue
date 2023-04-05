#pragma once

#include "domain.h"
#include "geo.h"
#include "transport_catalogue.h"
#include "svg.h"

#include <algorithm>
#include <string_view>
#include <vector>
#include <set>
#include <unordered_map>
#include <string>


inline const double EPSILON = 1e-6;


namespace sphere_projector {

    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding);

        svg::Point operator()(Coordinates coords) const {
            return {
                    (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_ = 0;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;

    private:
        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }
    };

    template <typename PointInputIt>
    SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                                     double max_width, double max_height, double padding) : padding_(padding) {
        if (points_begin == points_end) {
            return;
        }
        const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;
        const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }
        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

} // namespace sphere_projector

namespace map_renderer {

    struct CompStop {
        bool operator() (transport_catalogue::stop::Stop* lhs, transport_catalogue::stop::Stop* rhs) const;
    };



    struct RenderSettings {
        double width;
        double height;
        double padding;
        double line_width;
        double stop_radius;
        uint32_t bus_label_font_size;
        std::vector<double> bus_label_offset;
        uint32_t stop_label_font_size;
        std::vector<double> stop_label_offset;
        svg::Color underlayer_color;
        double underlayer_width;
        std::vector<svg::Color> color_palette;
    };


    class MapRenderer {
    public:
        MapRenderer(const RenderSettings&& settings) : settings_(std::move(settings)) {}

        void RenderMap(transport_catalogue::TransportCatalogue& transport_catalogue);

        std::string GetMapAsString() const;

        const RenderSettings& GetRenderSettings() const;

    private:
        const RenderSettings settings_;
        std::vector<Coordinates> geo_coordinates_;
        std::unordered_map<std::string_view, transport_catalogue::bus::Bus*> buses_;
        std::set<std::string_view> name_of_buses;
        std::set<transport_catalogue::stop::Stop*, CompStop> all_stops_;
        svg::Document map_;
        std::string map_as_string_;

    private:
        void AppendBuses(transport_catalogue::TransportCatalogue& transport_catalogue);

        void AppendCoordinates(transport_catalogue::TransportCatalogue& transport_catalogue);

        void ReadGeoCoordinatesFromStops();

        void BusNameSorting();

        void VisualizationRouteLines(const sphere_projector::SphereProjector& proj);

        void VisualizationRouteName(const sphere_projector::SphereProjector& proj);

        void AppendSubstrateForRoutes(std::string_view name, const svg::Point& point);

        void AppendTitleForRoutes(std::string_view name, const svg::Point& point, svg::Color color);

        void VisualizationRouteStops(const sphere_projector::SphereProjector& proj);

        void GetAllStops();

        void VisualizationStopName(const sphere_projector::SphereProjector& proj);

        void AppendSubstrateForNameStop(std::string_view name, const svg::Point& point);

        void AppendTitleForNameStop(std::string_view name, const svg::Point& point, svg::Color color);
    };

} // namespace map_renderer