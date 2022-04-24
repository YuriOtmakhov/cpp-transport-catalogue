#pragma once

#include <string>

#include "transport_catalogue.h"

namespace transport_catalogue {

class StatReader {

TransportCatalogue* const catalogue_;

public:
    explicit StatReader (TransportCatalogue* const trans_cat) : catalogue_(trans_cat) {
    };

    std::string GetStat(std::string_view str) const;

};

}
