#pragma once

#include <list>
#include <string>
#include <algorithm>

#include "transport_catalogue.h"

class InputReader {

struct QueryDate {
    bool is_stop;
    std::string name;
    std::string text;
};

std::list<QueryDate> querys_;
TransportCatalogue* const catalogue_;

public:
    explicit InputReader (TransportCatalogue* const trans_cat) : catalogue_(trans_cat) {
    }

    void AddQuery (std::string_view str);

    void PushQuery ();

    ~InputReader ();

};
