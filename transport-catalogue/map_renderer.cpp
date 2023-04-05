#include <sstream>

#include "map_renderer.h"


namespace map_renderer {

    using namespace std;
    using namespace sphere_projector;
    using namespace sphere_projector;
    using namespace transport_catalogue;

    bool CompStop::operator() (transport_catalogue::stop::Stop* lhs, transport_catalogue::stop::Stop* rhs)const{
        return lhs->name < rhs->name;
    }

    void MapRenderer::AppendBuses(TransportCatalogue& transport_catalogue) {
        buses_ = transport_catalogue.GetAllBuses();
        BusNameSorting();
    }

    void MapRenderer::AppendCoordinates(TransportCatalogue& transport_catalogue) {
        const std::unordered_map<std::string_view, transport_catalogue::stop::Stop*>& stops = transport_catalogue.GetAllStops();
        for (auto& [key, value] : stops) {
            if (!transport_catalogue.GetBuses(value).empty()){
                geo_coordinates_.push_back(value->coordinates);
            }
        }
    }

    void MapRenderer::BusNameSorting() {
        for (const auto& [key, value] : buses_) {
            name_of_buses.insert(key);
        }
    }

    void MapRenderer::RenderMap(TransportCatalogue& transport_catalogue) {
        AppendBuses(transport_catalogue);
        AppendCoordinates(transport_catalogue);
        const SphereProjector proj(geo_coordinates_.begin(), geo_coordinates_.end(), settings_.width, settings_.height, settings_.padding);
        VisualizationRouteLines(proj);
        VisualizationRouteName(proj);
        VisualizationRouteStops(proj);
        VisualizationStopName(proj);
        std::ostringstream stream;
        map_.Render(stream);
        map_as_string_ = stream.str();
    }

    std::string MapRenderer::GetMapAsString() const {
        return map_as_string_;
    }

    void MapRenderer::VisualizationRouteLines(const SphereProjector& proj) {
        int color_count = 0;
        for (auto& name : name_of_buses) {
            svg::Polyline polyline;
            auto& stops = buses_.at(name)->route;
            if (stops.empty()) {
                continue;
            }
            for (auto& stop : stops) {
                polyline.AddPoint(proj(stop->coordinates))
                        .SetFillColor(svg::NoneColor)
                        .SetStrokeColor(settings_.color_palette[color_count % settings_.color_palette.size()])
                        .SetStrokeWidth(settings_.line_width)
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            }
            if (!buses_.at(name)->circle) {
                for (auto it = stops.rbegin() + 1; it != stops.rend(); ++it) {
                    polyline.AddPoint(proj((*it)->coordinates))
                            .SetFillColor(svg::NoneColor)
                            .SetStrokeColor(settings_.color_palette[color_count % settings_.color_palette.size()])
                            .SetStrokeWidth(settings_.line_width)
                            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                }
            }
            ++color_count;
            map_.Add(move(polyline));
        }
    }

    void MapRenderer::VisualizationRouteName(const SphereProjector& proj) {
        int color_count = 0;
        for (auto& name : name_of_buses) {
            auto& stops = buses_.at(name)->route;
            if (stops.empty()) {
                continue;
            }
            if (buses_.at(name)->circle) {
                svg::Point point = proj(stops[0]->coordinates);
                AppendSubstrateForRoutes(name, point);
                AppendTitleForRoutes(name, point, settings_.color_palette[color_count % settings_.color_palette.size()]);
            } else {

                if (stops[0]->name == stops[stops.size() - 1]->name) {
                    svg::Point point_stop_1 = proj(stops[0]->coordinates);
                    AppendSubstrateForRoutes(name, point_stop_1);
                    AppendTitleForRoutes(name, point_stop_1, settings_.color_palette[color_count % settings_.color_palette.size()]);
                } else {
                    svg::Point point_stop_1 = proj(stops[0]->coordinates);
                    AppendSubstrateForRoutes(name, point_stop_1);
                    AppendTitleForRoutes(name, point_stop_1, settings_.color_palette[color_count % settings_.color_palette.size()]);
                    svg::Point point_stop_2 = proj(stops[stops.size() - 1]->coordinates);
                    AppendSubstrateForRoutes(name, point_stop_2);
                    AppendTitleForRoutes(name, point_stop_2, settings_.color_palette[color_count % settings_.color_palette.size()]);
                }
            }
            ++color_count;
        }
    }

    void MapRenderer::AppendSubstrateForRoutes(std::string_view name, const svg::Point& point) {
        svg::Text text;
        text.SetPosition(point)
                .SetOffset({settings_.bus_label_offset[0], settings_.bus_label_offset[1]})
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(string(name))
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeColor(settings_.underlayer_color);
        map_.Add(move(text));
    }

    void MapRenderer::AppendTitleForRoutes(std::string_view name, const svg::Point& point,svg::Color color) {
        svg::Text text;
        text.SetPosition(point)
                .SetFillColor(color)
                .SetOffset({settings_.bus_label_offset[0], settings_.bus_label_offset[1]})
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(string(name));
        map_.Add(move(text));
    }

    void MapRenderer::VisualizationRouteStops(const SphereProjector& proj) {
        svg::Circle circle;
        GetAllStops();
        for (auto& stop : all_stops_) {
            circle.SetCenter(proj(stop->coordinates))
                    .SetRadius(settings_.stop_radius)
                    .SetFillColor("white");
            map_.Add(move(circle));
        }
    }

    void MapRenderer::GetAllStops() {
        for (auto& name : name_of_buses) {
            auto& stops = buses_.at(name)->route;
            for (auto& stop : stops) {
                all_stops_.insert(stop);
            }
        }
    }

    void MapRenderer::VisualizationStopName(const SphereProjector& proj) {
        for (auto& stop : all_stops_) {
            AppendSubstrateForNameStop(stop->name, proj(stop->coordinates));
            AppendTitleForNameStop(stop->name, proj(stop->coordinates), "black");
        }
    }

    void MapRenderer::AppendSubstrateForNameStop(std::string_view name, const svg::Point& point) {
        svg::Text text;
        text.SetPosition(point)
                .SetOffset({settings_.stop_label_offset[0], settings_.stop_label_offset[1]})
                .SetFontSize(settings_.stop_label_font_size)
                .SetFontFamily("Verdana")
                .SetData(string(name))
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeColor(settings_.underlayer_color);
        map_.Add(move(text));
    }

    void MapRenderer::AppendTitleForNameStop(std::string_view name, const svg::Point& point, svg::Color color) {
        svg::Text text;
        text.SetPosition(point)
                .SetFillColor(color)
                .SetOffset({settings_.stop_label_offset[0], settings_.stop_label_offset[1]})
                .SetFontSize(settings_.stop_label_font_size)
                .SetFontFamily("Verdana")
                .SetData(string(name));
        map_.Add(move(text));
    }

    const RenderSettings& MapRenderer::GetRenderSettings() const {
        return settings_;
    }

} // namespace map_renderer