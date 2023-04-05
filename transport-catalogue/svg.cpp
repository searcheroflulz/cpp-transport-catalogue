#include "svg.h"

namespace svg {
    std::ostream& operator<<(std::ostream& out, StrokeLineCap value) {
        switch(value) {
            case StrokeLineCap::BUTT: {
                out << "butt";
                break;
            }
            case StrokeLineCap::ROUND: {
                out << "round";
                break;
            }
            case StrokeLineCap::SQUARE: {
                out << "square";
                break;
            }
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin value) {
        switch(value) {
            case StrokeLineJoin::ARCS: {
                out << "arcs";
                break;
            }
            case StrokeLineJoin::BEVEL: {
                out << "bevel";
                break;
            }
            case StrokeLineJoin::MITER: {
                out << "miter";
                break;
            }
            case StrokeLineJoin::MITER_CLIP: {
                out << "miter-clip";
                break;
            }
            case StrokeLineJoin::ROUND: {
                out << "round";
                break;
            }
        }
        return out;
    }

    std::ostream& operator<< (std::ostream& out, Color color) {
        visit([&out](auto value) {
            PrintRoots(out, value);
        }, color);
        return out;
    }

    void PrintRoots(std::ostream& out, std::monostate) {
        out << "none"sv;
    }

    void PrintRoots(std::ostream& out, std::string str) {
        out << str;
    }

    void PrintRoots(std::ostream& out, svg::Rgb rgb) {
        out << "rgb(" << static_cast<long unsigned int>(rgb.red) << "," << static_cast<long unsigned int>(rgb.green) << "," << static_cast<long unsigned int>(rgb.blue) << ")";
    }

    void PrintRoots(std::ostream& out, svg::Rgba rgba) {
        out << "rgba(" << static_cast<long unsigned int>(rgba.red) << "," << static_cast<long unsigned int>(rgba.green) << "," << static_cast<long unsigned int>(rgba.blue) << "," << rgba.opacity << ")";
    }

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

// ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center)  {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)  {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        this->RenderAttrs(out);
        out << "/>"sv;
    }

}