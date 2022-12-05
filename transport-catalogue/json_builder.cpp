#include "json_builder.h"

namespace json {

    KeyItemContext Builder::Key(std::string key) {
        KeyItemContext context (*this);
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
            throw std::logic_error("Can't add key");
        }
        auto& dict = std::get<Dict>(nodes_stack_.back()->GetValue());
        dict[key];
        key_ = key;
        return context;
    }

    Builder& Builder::Value(Node::Value value) {
        if (root_.GetValue().index() == 0) {
            root_.GetValue() = std::move(value);
            return *this;
        }
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
            auto& array = std::get<Array>(nodes_stack_.back()->GetValue());
            Node temp;
            temp.GetValue() = std::move(value);
            array.emplace_back(temp);
            if (temp.IsMap() || temp.IsArray()) {
                nodes_stack_.emplace_back(&array.back());
            }
            return *this;
        }
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap()) {
            if (key_) {
                auto &dict = std::get<Dict>(nodes_stack_.back()->GetValue());
                Node temp;
                temp.GetValue() = std::move(value);
                dict[*key_] = temp;
                if (temp.IsArray() || temp.IsMap()) {
                    nodes_stack_.emplace_back(&dict[*key_]);
                }
                key_.reset();
                return *this;
            }
            throw std::logic_error("Can't add value");
        }
        throw std::logic_error("Can't add value");
    }

    DictItemContext Builder::StartDict() {
        DictItemContext context(*this);
        if (root_.GetValue().index() == 0) {
            root_ = Dict();
            nodes_stack_.emplace_back(&root_);
            return context;
        }
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
            auto& array = std::get<Array>(nodes_stack_.back()->GetValue());
            array.emplace_back(Dict());
            nodes_stack_.emplace_back(&array.back());
            return context;
        }
        if (key_) {
            auto& dict = std::get<Dict>(nodes_stack_.back()->GetValue());
            dict[*key_] = Dict();
            nodes_stack_.emplace_back(&dict[*key_]);
            key_.reset();
            return context;
        }
        throw std::logic_error("Can't start Dict");
    }

    ArrayItemContext Builder::StartArray() {
        ArrayItemContext context(*this);
        if (root_.GetValue().index() == 0) {
            root_ = Array();
            nodes_stack_.emplace_back(&root_);
            return context;
        }
        if (key_) {
            auto& dict = std::get<Dict>(nodes_stack_.back()->GetValue());
            dict[*key_] = Array();
            nodes_stack_.emplace_back(&dict[*key_]);
            key_.reset();
            return context;
        }
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
            auto& array = std::get<Array>(nodes_stack_.back()->GetValue());
            array.emplace_back(Array());
            nodes_stack_.emplace_back(&array.back());
            return context;
        }
        throw std::logic_error("Can't start Array");
    }

    Builder& Builder::EndDict() {
        if (!nodes_stack_.empty() && !nodes_stack_.back()->IsMap()) {
            throw std::logic_error("Can't end Dict");
        }
        if (key_) {
            throw std::logic_error("Can't end Dict");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::EndArray() {
        if (!nodes_stack_.empty() && !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("Error end Array");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    json::Node Builder::Build() {
        if (root_.GetValue().index() == 0 || !nodes_stack_.empty()) {
            throw std::logic_error("Can't build Node");
        }
        return root_;
    }


    KeyItemContext DictItemContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    Builder& DictItemContext::EndDict() {
        return builder_.EndDict();
    }

    DictItemContext KeyItemContext::StartDict() {
        return builder_.StartDict();
    }

    ValueItemContextKey KeyItemContext::Value(Node::Value value) {
        ValueItemContextKey contex(builder_);
        builder_ = builder_.Value(std::move(value));
        return contex;
    }

    ArrayItemContext KeyItemContext::StartArray() {
        return builder_.StartArray();
    }

    DictItemContext ArrayItemContext::StartDict() {
        return builder_.StartDict();
    }

    ValueItemContextArray ArrayItemContext::Value(Node::Value value) {
        ValueItemContextArray contex(builder_);
        builder_ = builder_.Value(std::move(value));
        return contex;
    }

    ArrayItemContext ArrayItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& ArrayItemContext::EndArray() {
        return builder_.EndArray();
    }

    Builder& ValueItemContextKey::EndDict() {
        return builder_.EndDict();
    }

    KeyItemContext ValueItemContextKey::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    DictItemContext ValueItemContextArray::StartDict() {
        return builder_.StartDict();
    }

    ValueItemContextArray ValueItemContextArray::Value(Node::Value value) {
        ValueItemContextArray contex(builder_);
        builder_ = builder_.Value(std::move(value));
        return contex;
    }

    ArrayItemContext ValueItemContextArray::StartArray() {
        return builder_.StartArray();
    }

    Builder& ValueItemContextArray::EndArray() {
        return builder_.EndArray();
    }

}