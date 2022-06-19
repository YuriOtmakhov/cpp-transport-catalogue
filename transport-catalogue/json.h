#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using ValueType = std::variant<std::nullptr_t, bool, int, double, std::string, Array, Dict>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {

ValueType value_;

public:
   /* Реализуйте Node, используя std::variant */
    Node ();
    Node(std::nullptr_t);
    Node(Array array);
    Node(Dict map);
    Node(int value);
    Node(double value);
    Node(bool value);
    Node(std::string value);

    const ValueType& GetValue() const;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    bool AsBool() const;
    int AsInt() const;
    double AsDouble () const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

//    bool operator== (const Node& rhs) const;
//
//    bool operator!= (const Node& rhs) const;
};

bool operator== (const Node& lhs, const Node& rhs);

bool operator!= (const Node& lhs, const Node& rhs);

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator== (const Document& lhs, const Document& rhs);

bool operator!= (const Document& lhs, const Document& rhs);

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
