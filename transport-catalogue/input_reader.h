#pragma once

#include <list>
#include <string>

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

public:
    explicit InputReader (TransportCatalogue* const trans_cat) : catalogue_(trans_cat) {
    };

    void AddQuery (std::string_view str);

    void PushQuery ();

    ~InputReader ();

};

}
