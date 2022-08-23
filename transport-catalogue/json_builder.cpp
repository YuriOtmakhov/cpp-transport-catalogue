#include "json_builder.h"

using namespace json;
using std::literals::string_literals::operator""s;

Builder::KeyItemContext Builder::Key(std::string key) {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap())
        throw std::logic_error("value"s);

    nodes_stack_.emplace_back(new Node);
    nodes_stack_.back()->GetValue() = std::move(key);

    return  {*this };
}

Builder::BaseContext Builder::Value(ValueType node) {
    if (nodes_stack_.empty()){
        if (value_)
            throw std::logic_error("value"s);

        Node n_node;
        n_node.GetValue() =  node;
        value_ = std::move(n_node);

    }else if (nodes_stack_.back()->IsString()) {
        auto key = nodes_stack_.back()->AsString();
        delete nodes_stack_.back();
        nodes_stack_.pop_back();

        if (nodes_stack_.back()->IsMap())
            std::get<Dict>(nodes_stack_.back()->GetValue())[key].GetValue() = std::move(node);
        else
            throw std::logic_error("value"s);
    }else if  (nodes_stack_.back()->IsArray()) {
        std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back();
        std::get<Array>(nodes_stack_.back()->GetValue()).back().GetValue() = std::move(node);
    }else
        throw std::logic_error("value"s);

    return  {*this };
}

Builder::DictItemContext Builder::StartDict() {
    if (!nodes_stack_.empty())
        if (!nodes_stack_.back()->IsString() && !nodes_stack_.back()->IsArray())
            throw std::logic_error("StartDict"s);
    nodes_stack_.emplace_back(new Node(Dict{}) );
    return  {*this };
}

bool Builder::IsCorrectEnd () {

    Node node = *nodes_stack_.back();
    nodes_stack_.pop_back();
    if (nodes_stack_.empty())
        value_ = std::move(node);
    else if(nodes_stack_.back()->IsString()) {
        std::string key = nodes_stack_.back()->AsString();
        delete nodes_stack_.back();
        nodes_stack_.pop_back();
        if (nodes_stack_.back()->IsMap())
            std::get<Dict>(nodes_stack_.back()->GetValue())[key] = std::move(node);
        else
            return false;
    }else if(nodes_stack_.back()->IsArray()) {
        std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back();
        std::get<Array>(nodes_stack_.back()->GetValue()).back() = std::move(node);
    }else
        return false;

    return true;
}

Builder::BaseContext Builder::EndDict() {
    if (nodes_stack_.empty())
        throw std::logic_error("EndDict"s);
    if ( !nodes_stack_.back()->IsMap())
        throw std::logic_error("EndDict"s);

    if (!IsCorrectEnd ())
        throw std::logic_error("EndDict"s);

    return  {*this };
}

Builder::ArrayItemContext Builder::StartArray(){
    if (!nodes_stack_.empty())
        if (!nodes_stack_.back()->IsString() && !nodes_stack_.back()->IsArray())
            throw std::logic_error("StartArray"s);
    nodes_stack_.emplace_back(new Node(Array{}));
    return  {*this };
}

Builder::BaseContext Builder::EndArray() {
    if (nodes_stack_.empty())
        throw std::logic_error("EndArray"s);
    if ( !nodes_stack_.back()->IsArray())
        throw std::logic_error("EndArray"s);

    if (!IsCorrectEnd ())
        throw std::logic_error("EndArray"s);

    return  {*this };
}

const Node& Builder::Build() const {
    if (!nodes_stack_.empty())
        throw std::logic_error("Build"s);
    if (!value_)
        throw std::logic_error("Build"s);
    return *value_;
}

Builder::BaseContext::BaseContext(Builder& builder): builder_(&builder) {
}

Builder::BaseContext Builder::BaseContext::Value(ValueType node) {
    return {builder_->Value(node)};
}

Builder::BaseContext Builder::BaseContext::EndDict(){
    return builder_->EndDict();
}

Builder::ArrayItemContext Builder::BaseContext::StartArray(){
    return builder_->StartArray();
}

Builder::BaseContext Builder::BaseContext::EndArray() {
    return builder_->EndArray();
}

Builder::KeyItemContext Builder::BaseContext::Key(std::string key) {
    return builder_->Key(key);
}

Builder::DictItemContext Builder::BaseContext::StartDict() {
    return builder_->StartDict();
}

const Node& Builder::BaseContext::Build() {
    return builder_->Build();
}

Builder::ValueArrayItemContext Builder::ArrayItemContext::Value(ValueType node)  {
    return static_cast<ValueArrayItemContext>(BaseContext::Value(node));
}

Builder::ValueKeyItemContext Builder::KeyItemContext::Value(ValueType node) {
    return static_cast<ValueKeyItemContext>(BaseContext::Value(node));
}

Builder::ValueKeyItemContext::ValueKeyItemContext (BaseContext base) : BaseContext(base) {
}

Builder::ValueArrayItemContext::ValueArrayItemContext (BaseContext base) : BaseContext(base) {
}

Builder::ValueArrayItemContext Builder::ValueArrayItemContext::Value(ValueType node){
    return static_cast<ValueArrayItemContext>(BaseContext::Value(node));
}
