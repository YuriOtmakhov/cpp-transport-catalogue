#pragma once

#include <vector>
#include <string>
#include <optional>

#include "json.h"

namespace json {


class Builder {
std::optional<Node> value_;
std::vector<Node*> nodes_stack_;

class DictItemContext;
class ArrayItemContext;
class KeyItemContext;
class ValueKeyItemContext;
class ValueArrayItemContext;

class BaseContext {
Builder* builder_;

public:

    BaseContext(Builder& builder);

    BaseContext Value(ValueType node);
    BaseContext EndDict();
    ArrayItemContext StartArray();
    BaseContext EndArray();
    KeyItemContext Key(std::string key);
    DictItemContext StartDict();

    const Node& Build();

};

class DictItemContext: public BaseContext {

BaseContext Value(ValueType node) = delete;
ArrayItemContext StartArray()= delete;
BaseContext EndArray()= delete;
DictItemContext StartDict()= delete;
const Node& Build() = delete;

};

class ArrayItemContext: public BaseContext {

BaseContext EndDict() = delete;
KeyItemContext Key(std::string key) = delete;
const Node& Build() = delete;

public:
    ValueArrayItemContext Value(ValueType node);
};

class KeyItemContext: public BaseContext {

BaseContext EndDict()= delete;
BaseContext EndArray()= delete;
KeyItemContext Key(std::string key)= delete;
const Node& Build() = delete;

public:
    ValueKeyItemContext Value(ValueType node);
};

class ValueKeyItemContext: public BaseContext {

BaseContext Value(ValueType node)= delete;
ArrayItemContext StartArray()= delete;
BaseContext EndArray()= delete;
DictItemContext StartDict()= delete;
const Node& Build() = delete;

public:
    ValueKeyItemContext (BaseContext base);
};

class ValueArrayItemContext: public BaseContext {

BaseContext EndDict()= delete;
KeyItemContext Key(std::string key)= delete;
const Node& Build() = delete;

public:
    ValueArrayItemContext (BaseContext base);
    ValueArrayItemContext Value(ValueType node);
};

public:

    KeyItemContext Key(std::string key);
    BaseContext Value(ValueType node);
    DictItemContext StartDict();
    BaseContext EndDict();
    ArrayItemContext StartArray();
    BaseContext EndArray();
    const Node& Build() const;

};

}
