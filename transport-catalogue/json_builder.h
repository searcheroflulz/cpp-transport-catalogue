#pragma once
#include <optional>
#include <deque>
#include <utility>

#include "json.h"



namespace json {
    class DictItemContext;

    class ArrayItemContext;

    class KeyItemContext;

    class ValueItemContextArray;

    class ValueItemContextKey;

    class Builder {
    public:

        Builder() = default;

        KeyItemContext Key(std::string key);

        Builder& Value(Node::Value value);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        Builder& EndDict();

        Builder& EndArray();

        json::Node Build();

    private:
        json::Node root_;
        std::deque<Node*> nodes_stack_;
        std::optional<std::string> key_;
    };

    class DictItemContext : public Builder {
    public:
        explicit DictItemContext(Builder& builder) : builder_(builder) {}

        KeyItemContext Key(std::string key);

        Builder& Value(Node::Value value) = delete;

        DictItemContext StartDict() = delete;

        ArrayItemContext StartArray() = delete;

        Builder& EndDict();

        Builder& EndArray() = delete;

        json::Node Build() = delete;
    private:
        Builder& builder_;
    };

    class KeyItemContext : public Builder {
    public:
        explicit KeyItemContext(Builder& builder) : builder_(builder) {}

        DictItemContext StartDict();

        Builder& EndDict() = delete;

        KeyItemContext Key(std::string key) = delete;

        ValueItemContextKey Value(Node::Value value);

        ArrayItemContext StartArray();

        Builder& EndArray() = delete;

        json::Node& Build() = delete;

    private:
        Builder& builder_;
    };

    class ArrayItemContext : public Builder {
    public:
        explicit ArrayItemContext(Builder& builder) : builder_(builder) {}

        Builder& Key(std::string key) = delete;

        Builder& EndDict() = delete;

        DictItemContext StartDict();

        ValueItemContextArray Value(Node::Value value);

        ArrayItemContext StartArray();

        Builder& EndArray();

        json::Node Build() = delete;
    private:
        Builder& builder_;
    };

    class ValueItemContextKey : public Builder {
    public:
        explicit ValueItemContextKey(Builder& builder) : builder_(builder) {}

        DictItemContext StartDict() = delete;

        Builder& EndDict();

        KeyItemContext Key(std::string key);

        Builder& Value(Node::Value value) = delete;

        ArrayItemContext StartArray() = delete;

        Builder& EndArray() = delete;

        json::Node& Build() = delete;

    private:
        Builder& builder_;
    };

    class ValueItemContextArray : public Builder {
    public:
        explicit ValueItemContextArray(Builder& builder) : builder_(builder) {}

        DictItemContext StartDict();

        Builder& EndDict() = delete;

        KeyItemContext Key(std::string key) = delete;

        ValueItemContextArray Value(Node::Value value);

        ArrayItemContext StartArray();

        Builder& EndArray();

        json::Node& Build() = delete;

    private:
        Builder& builder_;
    };
}