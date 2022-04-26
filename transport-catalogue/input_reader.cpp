#include "input_reader.h"

#include <list>
#include <string>
#include <algorithm>

using namespace transport_catalogue;

InputReader::~InputReader () {
    PushQuery();
}

void InputReader::AddQuery (std::string_view str) {
    auto ptr_separator = str.find(':');
    if (bool is_stop = str.substr(0,4) == "Stop")
        querys_.push_front({ is_stop , static_cast<std::string>(str.substr(5, ptr_separator-5)),static_cast<std::string>(str.substr(ptr_separator+2))});
    else
        querys_.push_back({ is_stop , static_cast<std::string>(str.substr(4, ptr_separator-4)),static_cast<std::string>(str.substr(ptr_separator+2))});
}

std::list<std::pair<std::string_view, size_t>> InputReader::ParsQueryDistance (std::string_view query) {
    std::list<std::pair<std::string_view, size_t>> distance;
    size_t ptr_r = 0;
    while(ptr_r != query.size()) {
        auto ptr_l = ptr_r;
        auto ptr_m = query.find('m', ptr_l + 2);
        ptr_r = query.find(',', ptr_l + 2);

        if (ptr_r == std::string::npos)
            ptr_r = query.size();

        distance.push_back( std::make_pair( query.substr(ptr_m + 5, ptr_r - ptr_m - 5),
                                            std::stoul(static_cast<std::string>(query.substr(ptr_l + 2, ptr_m - ptr_l)))
                                            )
                            );
    }

    return distance;
}

std::vector<std::string_view> InputReader::ParsQueryRoute (std::string_view query) {
    std::vector<std::string_view> route;

    int ptr_l = -2;
    for(;;) {
        auto ptr_r = query.find_first_of("->", ptr_l + 2);
        if (ptr_r != std::string::npos) {
            route.push_back( std::string_view(query).substr(ptr_l + 2, ptr_r - ptr_l - 3) );
            ptr_l = ptr_r;
        } else {
            route.push_back( std::string_view(query).substr(ptr_l + 2) );
            break;
        }
    }
    if (query[ptr_l] == '-')
        for (int i = route.size()-2; i>=0 ; --i)
            route.push_back(route[i]);
    return route;
}

void InputReader::PushQuery () {
    auto It_query = querys_.begin();

    std::list<std::pair<std::string_view, std::list<std::pair<std::string_view, size_t>>>> distance_to_stop;

    for ( ; It_query->is_stop; ++It_query) {

        auto ptr_l = It_query->text.find(',');
        auto ptr_r = It_query->text.find(',', ptr_l + 2);
        if (ptr_r == std::string::npos)
            ptr_r = It_query->text.size();

        catalogue_->AddStop( It_query->name,
                            std::stod(It_query->text.substr(0, ptr_l )),
                            std::stod(It_query->text.substr(ptr_l + 2, ptr_r - ptr_l))
                                );

        distance_to_stop.push_back({It_query->name, ParsQueryDistance(std::string_view(It_query->text).substr(ptr_r))});

    }

    for (const auto& distance : distance_to_stop)
        catalogue_->AddDistance(distance.first, distance.second);

    for ( auto It_end =  querys_.end(); It_query != It_end; ++It_query)
        catalogue_->AddBus(It_query->name, ParsQueryRoute(It_query->text));
}
