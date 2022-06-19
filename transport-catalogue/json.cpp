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
    getline(input, line, ',');

    if ( (/*IsEscape(line[0]) ||*/ (line[0] != '-') ) && ( line[0]-'0' < 0 || 9 < line[0]-'0' ))
        throw ParsingError("Parsing error"s);

    while (IsEscape(line.back()))
        line.pop_back();
    if (line.back() == ']' || line.back() == '}') {
        input.putback(line.back());
        line.pop_back();
    }

    if (string::npos == line.find_first_of(".Ee"s))
        return Node(stoi(line));

    return Node(stod(line));
}

Node LoadString(istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(s);

}

Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c && c != '}';) {
//        if (c == ',') {
//            input >> c;
//        }
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
//    if (!input)
//        throw ParsingError("EOF"s);
    char c;
    do {
        input >> c;
    } while ( IsEscape( c) );
//    cout<<"c1 - "<<c<<endl;
//    char c2;
//    input >> c2;
//    cout<<c2<<endl;
//    input.putback(c2);
//    if (!input)
//        throw ParsingError("String parsing error");
//    cout<<c<<endl;
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
Node::Node() : value_(nullptr) {
}

Node::Node(nullptr_t) : value_(nullptr) {
}

Node::Node(Array array) : value_(move(array)) {
}

Node::Node(Dict map) : value_(move(map)) {
}

Node::Node(int value) : value_(move(value)) {
}

Node::Node(string value) : value_(move(value)) {
}

Node::Node (double value) : value_(move(value)) {
}

Node::Node (bool value) : value_(move(value)) {
}

const ValueType& Node::GetValue() const {
    return value_;
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}

bool Node::IsDouble() const {
    return (std::holds_alternative<int>(value_)) || (std::holds_alternative<double>(value_));
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(value_);
}

bool Node::AsBool() const {
    if (!IsBool())
        throw std::logic_error ("is not bool");
    return std::get<bool>(value_);
}

double Node::AsDouble() const {
    if (!IsDouble())
        throw std::logic_error ("is not double");

    return (IsPureDouble() ? std::get<double>(value_) : static_cast<double>(std::get<int>(value_)) );
}

const Array& Node::AsArray() const {
    if (!IsArray())
        throw std::logic_error ("is not array");
    return std::get<Array>(value_);
}

const Dict& Node::AsMap() const {
    if (!IsMap())
        throw std::logic_error ("is not map");
    return std::get<Dict>(value_);
}

int Node::AsInt() const {
    if (!IsInt())
        throw std::logic_error ("is not int");
    return std::get<int>(value_);
}

const string& Node::AsString() const {
    if (!IsString())
        throw std::logic_error ("is not string");
    return std::get<std::string>(value_);
}

//bool Node::operator== (const Node& rhs) const {
//    return value_ == rhs.value_;
//}
//
//bool Node::operator!= (const Node& rhs) const {
//    return !(*this == rhs );
//}

bool operator== (const Node& lhs, const Node& rhs){
    return lhs.GetValue() == rhs.GetValue();
}

bool operator!= (const Node& lhs, const Node& rhs){
    return !(lhs == rhs );
}

bool operator== (const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
};

bool operator!= (const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
};

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
    //cout<<str<<endl;
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
