#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        PrintContext Indented() const {
            return {out, indent_step, indent_step + indent};
        }
    };

    class Node;
// Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final {
    public:

        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;
        /* Реализуйте Node, используя std::variant */
        Node(std::nullptr_t value) : value_(value) {}
        Node(Array val) : value_(std::move(val)){}
        Node(Dict val) : value_(std::move(val)) {}
        Node(bool val) : value_(val){}
        Node(int val) : value_(val){}
        Node(double val) : value_(val) {}
        Node(std::string val) : value_(std::move(val)) {}

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        const std::string& AsString() const;


        bool IsInt() const {
            return std::holds_alternative<int>(value_);
        }
        bool IsDouble() const {
            return IsInt() || IsPureDouble();
        }
        bool IsPureDouble() const {
            return std::holds_alternative<double>(value_);
        }
        bool IsBool() const {
            return std::holds_alternative<bool>(value_);
        }
        bool IsString() const {
            return std::holds_alternative<std::string>(value_);
        }
        bool IsNull() const {
            return std::holds_alternative<std::nullptr_t>(value_);
        }
        bool IsArray() const {
            return std::holds_alternative<Array>(value_);
        }
        bool IsMap() const {
            return std::holds_alternative<Dict>(value_);
        }

        bool operator==(const Node& rhs) const {
            return GetValue() == rhs.GetValue();
        }

        const Value& GetValue() const {
            return value_;
        }

        Value& GetValue() {
            return value_;
        }

        double AsDouble() const {
            if (!IsDouble()) {
                throw std::logic_error("This is not double");
            }
            return IsPureDouble() ? std::get<double>(value_) : AsInt();
        }

        bool AsBool() const {
            if (!IsBool()) {
                throw std::logic_error("This is not bool");
            }
            return std::get<bool>(value_);
        }
    private:
        Value value_;
    };

    inline bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    inline bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    void PrintNode(const Node& node, const PrintContext& ctx);

}  // namespace json