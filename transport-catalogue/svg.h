#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>
#include <deque>
#include <optional>
#include <variant>

using namespace std::literals;

namespace svg {


    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineCap value);
    std::ostream& operator<<(std::ostream& out, StrokeLineJoin value);


    struct Rgb {
        Rgb(long unsigned int red_in = 0, long unsigned int green_in = 0, long unsigned int blue_in = 0) : red(red_in), green(green_in), blue(blue_in) {}

        long unsigned int red;
        long unsigned int green;
        long unsigned int blue;
    };

    struct Rgba {
        Rgba(long unsigned int red_in = 0, long unsigned int green_in = 0, long unsigned int blue_in = 0, double opacity_in = 1.0)
                : red(red_in), green(green_in), blue(blue_in), opacity (opacity_in) {}

        long unsigned int red;
        long unsigned int green;
        long unsigned int blue;
        double opacity;
    };

    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
    inline const std::string NoneColor{"none"};
    std::ostream& operator<< (std::ostream& out, svg::Color color);
    void PrintRoots(std::ostream& out, std::monostate);
    void PrintRoots(std::ostream& out, std::string);
    void PrintRoots(std::ostream& out, svg::Rgb);
    void PrintRoots(std::ostream& out, svg::Rgba);


    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_line_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = line_join;
            return AsOwner();
        }
    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }
        std::optional<double> stroke_line_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (stroke_line_width_) {
                out << " stroke-width=\""sv << *stroke_line_width_ << "\""sv;
            }
            if (stroke_line_cap_) {
                out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
            }
            if (stroke_line_join_) {
                out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
            }
        }
    };


    struct Point {
        Point() = default;
        Point(double x, double y)
                : x(x)
                , y(y) {
        }
        double x = 0;
        double y = 0;
    };

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
    struct RenderContext {
        RenderContext(std::ostream& out)
                : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
                : out(out)
                , indent_step(indent_step)
                , indent(indent) {
        }

        RenderContext Indented() const {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class ObjectContainer {
    public:
        virtual ~ObjectContainer() = default;
        template <typename Obj>
        void Add(Obj object) {
            //objects_.emplace_back(std::make_unique<Obj>(std::move(object)));
            auto ptr = std::make_unique<Obj>(std::move(object));
            AddPtr(std::move(ptr));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    };

    class Drawable {
    public:
        virtual ~Drawable() = default;
        virtual void Draw(svg::ObjectContainer& container) const = 0;
    };
/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
    class Polyline final : public Object, public PathProps<Polyline>{
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point) {
            points_.emplace_back(point);
            return *this;
        }

        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */

    private:
        void RenderObject(const RenderContext& context) const override {
            auto& out = context.out;
            out << "<polyline points=\"";
            if (points_.empty()) {
                out << "\"/>";
                return;
            }
            for (size_t i = 0; i != points_.size(); i++) {
                if (i == points_.size() - 1) {
                    out << points_[i].x << "," << points_[i].y << "\"";
                    break;
                }
                out << points_[i].x << "," << points_[i].y << " ";
            }
            this->RenderAttrs(out);
            out << "/>";
        }
        std::deque<Point> points_;
    };

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
    class Text final : public Object, public PathProps<Text>{
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos) {
            pos_ = pos;
            return *this;
        }

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset) {
            offset_ = offset;
            return *this;
        }

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size) {
            font_size_ = size;
            return *this;
        }

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family) {
            font_family_ = std::move(font_family);
            return *this;
        }

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight) {
            font_weight_ = std::move(font_weight);
            return *this;
        }

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data) {
            text_ = std::move(data);
            return *this;
        }
    private:
        Point pos_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string text_;

        std::string ProccesText () const {
            std::string result;
            std::string quot = "&quot;";
            std::string apos = "&apos;";
            std::string lt = "&lt;";
            std::string gt = "&gt;";
            std::string amp = "&amp;";
            for (size_t i = 0; i != text_.size(); i++) {
                char temp = text_[i];

                if (temp == '"') {
                    result.insert(result.size(), quot);
                    continue;
                }
                if (temp == '\'') {
                    result.insert(result.size(), apos);
                    continue;
                }
                if (temp == '<') {
                    result.insert(result.size(), lt);
                    continue;
                }
                if (temp == '>') {
                    result.insert(result.size(), gt);
                    continue;
                }
                if (temp == '&') {
                    result.insert(result.size(), amp);
                    continue;
                }
                result.push_back(temp);
            }
            return result;
        }
        void RenderObject(const RenderContext& context) const {
            auto& out = context.out;
            out << "<text";
            RenderAttrs(out);
            out << " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\"";
            out << " dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\"";
            out << " font-size=\"" << font_size_ << "\"";
            if (!font_family_.empty()){
                out << " font-family=\"" << font_family_ << "\"";
            }
            if (!font_weight_.empty())   {
                out << " font-weight=\"" << font_weight_ << "\"";
            }
            out << ">" << text_ << "</text>"sv;
        }
        // Прочие данные и методы, необходимые для реализации элемента <text>
    };

    class Document : public ObjectContainer{
    public:
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override {
            objects_.emplace_back(std::move(obj));
        }

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const {
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
            out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;
            for (auto& obj: objects_) {
                obj->Render(out);
            }
            out << "</svg> " << std::endl;
        }

        // Прочие методы и данные, необходимые для реализации класса Document
    private:
        std::deque<std::unique_ptr<Object>> objects_;
    };
}