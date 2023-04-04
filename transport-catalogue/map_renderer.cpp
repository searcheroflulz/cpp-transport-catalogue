#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace map_renderer {
    void MapRenderer::LoadBuses(const std::unordered_map<std::string_view, transport_catalogue::bus::Bus*>& buses) {
        buses_ = buses;
        for (auto& bus: buses_) {
            bus_names_.insert(bus.first);
        }
    }

    void MapRenderer::LoadCoordinates(transport_catalogue::TransportCatalogue& catalogue) {
        const auto stops = &catalogue.GetAllStops();
        for (const auto& stop: *stops) {
            if (!catalogue.GetBuses(stop.second).empty()){
                coordinates_.push_back(stop.second->coordinates);
                correct_stops_[stop.first] = stop.second;
            }
        }
    }

    void MapRenderer::Render(std::ostream& out) {
        const SphereProjector proj(coordinates_.begin(), coordinates_.end(), settings_.width, settings_.height, settings_.padding);
        DrawRoutes(proj);
        DrawRouteName(proj);
        DrawStops(proj);
        DrawStopName(proj);
        render_.Render(out);
    }

    std::string MapRenderer::Output() {
        std::ostringstream out;
        Render(out);
        return out.str();
    }

    void MapRenderer::DrawRoutes(const SphereProjector& proj) {
        int color_count = 0;
        for (auto& name: bus_names_) {
            svg::Polyline polyline;
            auto& stops = buses_.at(name)->route;
            if (stops.empty()) {
                continue;
            }
            for (auto& stop: stops) {
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
            render_.Add(std::move(polyline));
        }
    }

    void MapRenderer::DrawRouteName(const SphereProjector& proj) {
        int color_count = 0;
        for (auto& name: bus_names_) {
            auto& stops = buses_.at(name)->route;
            if (stops.empty()) {
                continue;
            }
            if (buses_.at(name)->circle) {
                svg::Point point = proj(stops[0]->coordinates);
                LoadSubstrateForRoutes(name, point);
                LoadTitleForRoutes(name, point, settings_.color_palette[color_count % settings_.color_palette.size()]);
            } else {
                if (stops[0]->name == stops[stops.size() - 1]->name) {
                    svg::Point point_stop_1 = proj(stops[0]->coordinates);
                    LoadSubstrateForRoutes(name, point_stop_1);
                    LoadTitleForRoutes(name, point_stop_1, settings_.color_palette[color_count % settings_.color_palette.size()]);
                } else {
                    svg::Point point_stop_1 = proj(stops[0]->coordinates);
                    LoadSubstrateForRoutes(name, point_stop_1);
                    LoadTitleForRoutes(name, point_stop_1, settings_.color_palette[color_count % settings_.color_palette.size()]);
                    svg::Point point_stop_2 = proj(stops[stops.size() - 1]->coordinates);
                    LoadSubstrateForRoutes(name, point_stop_2);
                    LoadTitleForRoutes(name, point_stop_2, settings_.color_palette[color_count % settings_.color_palette.size()]);
                }
            }
            ++color_count;
        }
    }

    void MapRenderer::LoadSubstrateForRoutes(std::string_view name, const svg::Point& point) {
        svg::Text text;
        text.SetPosition(point)
                .SetOffset({settings_.bus_label_offset[0], settings_.bus_label_offset[1]})
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(std::string(name))
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeColor(settings_.underlayer_color);
        render_.Add(std::move(text));
    }

    void MapRenderer::LoadTitleForRoutes(std::string_view name, const svg::Point& point, svg::Color color) {
        svg::Text text;
        text.SetPosition(point)
                .SetFillColor(std::move(color))
                .SetOffset({settings_.bus_label_offset[0], settings_.bus_label_offset[1]})
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(std::string(name));
        render_.Add(std::move(text));
    }

    void MapRenderer::DrawStops(const SphereProjector& proj) {
        svg::Circle circle;
        for (auto& stop: correct_stops_) {
            circle.SetCenter(proj(stop.second->coordinates))
                    .SetRadius(settings_.stop_radius)
                    .SetFillColor("white");
            render_.Add(std::move(circle));
        }
    }

    void MapRenderer::DrawStopName(const SphereProjector& proj) {
        for (auto& stop: correct_stops_) {
            LoadSubstrateForNameStop(stop.first, proj(stop.second->coordinates));
            LoadTitleForNameStop(stop.first, proj(stop.second->coordinates), "black");
        }
    }


    void MapRenderer::LoadSubstrateForNameStop(std::string_view name, const svg::Point& point) {
        svg::Text text;
        text.SetPosition(point)
                .SetOffset({settings_.stop_label_offset[0], settings_.stop_label_offset[1]})
                .SetFontSize(settings_.stop_label_font_size)
                .SetFontFamily("Verdana")
                .SetData(std::string(name))
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeColor(settings_.underlayer_color);
        render_.Add(std::move(text));
    }

    void MapRenderer::LoadTitleForNameStop(std::string_view name, const svg::Point& point, svg::Color color) {
        svg::Text text;
        text.SetPosition(point)
                .SetFillColor(std::move(color))
                .SetOffset({settings_.stop_label_offset[0], settings_.stop_label_offset[1]})
                .SetFontSize(settings_.stop_label_font_size)
                .SetFontFamily("Verdana")
                .SetData(std::string(name));
        render_.Add(std::move(text));
    }

    MapRenderSettings & MapRenderer::GetRenderSettings() const {
        return settings_;
    }
}