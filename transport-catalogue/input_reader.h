#pragma once

#include <list>
#include <string>
#include <vector>

#include "transport_catalogue.h"

namespace transport_catalogue {

class InputReader {

struct QueryDate {
    bool is_stop;
    std::string name;
    std::string text;
};

std::list<QueryDate> querys_;
TransportCatalogue* const catalogue_;

std::list<std::pair<std::string_view, size_t>> ParsQueryDistance (std::string_view query);
std::vector<std::string_view> ParsQueryRoute (std::string_view query);

public:
    explicit InputReader (TransportCatalogue* const trans_cat) : catalogue_(trans_cat) {
    };

    void AddQuery (std::string_view str);

    void PushQuery ();

    ~InputReader ();

};

}
