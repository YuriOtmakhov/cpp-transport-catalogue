#include "json.h"

using namespace std;
using namespace std::literals;

namespace json {

namespace {

inline bool IsEscape (char ch) {
    return ch == '\n' || ch == '\r' || ch == '\\' || ch == '\t' || ch == ' ';
}

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (input.eof())
        throw ParsingError("Array parsing error"s);

    return Node(move(result));
}

Node LoadNumber(istream& input) {
    string line;
    char ch;
    for(; input >> ch && ch != ',' && ch != ']' && ch != '}';)
        line+=ch;
    if (ch != ',')
        input.putback(ch);

    if ( (line[0] != '-') && ( line[0]-'0' < 0 || 9 < line[0]-'0' ))
        throw ParsingError("Number parsing error: "s+line);

    while (IsEscape(line.back()))
        line.pop_back();

    if (string::npos == line.find_first_of(".Ee"s))
        return Node(stoi(line));

    return Node(stod(line));
}

Node LoadString(istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string str;
    while (true) {
        if (it == end)
            throw ParsingError("String parsing error"s);

        if (*it == '"') {
            ++it;
            break;
        } else if (*it == '\\') {
            ++it;
            if (it == end)
                throw ParsingError("String parsing error"s);

            switch (*it) {
                case 'n':
                    str.push_back('\n');
                    break;
                case 't':
                    str.push_back('\t');
                    break;
                case 'r':
                    str.push_back('\r');
                    break;
                case '"':
                    str.push_back('"');
                    break;
                case '\\':
                    str.push_back('\\');
                    break;
                default:
                    throw ParsingError("Unrecognized escape sequence \\"s + *it);
            }
        } else if (*it == '\n' || *it == '\r')
            throw ParsingError("Unexpected end of line"s);
        else
            str.push_back(*it);

        ++it;
    }

    return Node(str);

}

Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c && c != '}';) {

        if (c != ',') {
            input.putback(c);
        }
        string key = LoadNode(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    if (input.eof())
        throw ParsingError("Map parsing error"s);

    return Node(move(result));
}

Node LoadNull(istream& input) {
    char str[4];
    input.get(str,4,',');
    if (str != "ull"s)
        throw ParsingError("Null parsing error"s);
    return Node();
}

Node LoadBool(istream& input) {

    char ch;
    input >> ch;
    switch(ch) {
        case 'r': {
            char str[3];
            input.get(str,3,',');
            if (str != "ue"s)
                 throw ParsingError("Bool parsing error"s);
            else return Node(true);
            }
        case 'a': {
            char str[4];
            input.get(str,4,',');
            if (str != "lse"s)
                 throw ParsingError("Bool parsing error"s);
            else return Node(false);
            }
        default:
            throw ParsingError("Bool parsing error"s);
    }

}

Node LoadNode(istream& input) {

    char c;
    do {
        input >> c;
    } while ( IsEscape( c) );

    switch (c) {
        case '[':
            return LoadArray(input);
        case '{':
            return LoadDict(input);
        case '"':
            return LoadString(input);
        case 'n':
            return LoadNull(input);
        case 't':
        case 'f':
            return LoadBool(input);
        default :
            input.putback(c);
            return LoadNumber(input);
    }
}

}
  // namespace

const ValueType& Node::GetValue() const {
    return *this;
}

ValueType& Node::GetValue() {
    return *this;
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(*this);
}

bool Node::IsDouble() const {
    return (std::holds_alternative<int>(*this)) || (std::holds_alternative<double>(*this));
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(*this);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(*this);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(*this);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(*this);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(*this);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(*this);
}

bool Node::AsBool() const {
    if (!IsBool())
        throw std::logic_error ("is not bool"s);
    return std::get<bool>(*this);
}

double Node::AsDouble() const {
    if (!IsDouble())
        throw std::logic_error ("is not double"s);

    return (IsPureDouble() ? std::get<double>(*this) : static_cast<double>(std::get<int>(*this)) );
}

const Array& Node::AsArray() const {
    if (!IsArray())
        throw std::logic_error ("is not array"s);
    return std::get<Array>(*this);
}

const Dict& Node::AsMap() const {
    if (!IsMap())
        throw std::logic_error ("is not map"s);
    return std::get<Dict>(*this);
}

int Node::AsInt() const {
    if (!IsInt())
        throw std::logic_error ("is not int"s);
    return std::get<int>(*this);
}

const string& Node::AsString() const {
    if (!IsString())
        throw std::logic_error ("is not string"s);
    return std::get<std::string>(*this);
}

bool operator== (const Node& lhs, const Node& rhs){
    return lhs.GetValue() == rhs.GetValue();
}

bool operator!= (const Node& lhs, const Node& rhs){
    return !(lhs == rhs );
}

bool operator== (const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

bool operator!= (const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void PrintNode(const Node& node, std::ostream& out);

template <typename Value>
void PrintValue(Value value, std::ostream& out) {
    out << value;
}

void PrintValue(const string& str, std::ostream& out) {
    out<<"\""s;
    for (char letter : str)
        switch (letter ) {
            case '\n' :
                out<<"\\n"s; break;
            case '\r' :
                out<<"\\r"s; break;
            case '\\' :
                out<<"\\\\"s; break;
            case '\t' :
                out<<"\t"s; break;
            case '\"' :
                out<<"\\\""s; break;
            default:
                out<<letter;
    }
    out<<"\""s<<flush;
}

void PrintValue(bool flag, std::ostream& out) {
    out << (flag? "true"sv : "false"sv);
}

void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"sv;
}

void PrintValue(Array array, std::ostream& out) {
    out << '[';
    bool flag = false;
    for (auto& item : array) {
        if (flag)
            out << ", ";
        PrintNode(item, out);
        flag = true;
    }
    out << ']';
}

void PrintValue(Dict map, std::ostream& out) {
    out << '{';
    bool flag = false;
    for (auto& [key, value] : map) {
        if (flag)
            out << ", ";
        out << '\"'
            << key
            << "\": ";
        PrintNode(value, out);
        flag = true;
    }
    out << '}';
}

void PrintNode(const Node& node, std::ostream& out) {
    std::visit(
        [&out](const auto& value){ PrintValue(value, out); },
        node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);
}

}  // namespace json
