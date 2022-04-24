#include "input_reader.h"

#include <list>
#include <string>
#include <algorithm>

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

void InputReader::PushQuery () {
    for (auto& query : querys_) {
        if (query.is_stop) {
            auto ptr_l = query.text.find(',');
            auto ptr_r = query.text.find(',', ptr_l+1);
            catalogue_->AddStop( query.name,
                                std::stod(query.text.substr(0, ptr_l )),
                                std::stod(query.text.substr(ptr_l+1, ((ptr_r != std::string::npos) ? ptr_r : (query.text.size() - 1) )  -ptr_l))
                                );

            if (ptr_r != std::string::npos) {
                query.text.erase(0,ptr_r+2);
            } else {
                query.text = {};
            }
        } else {
            std::vector<std::string_view> route;
            size_t ptr_l = -2;
            for(;;) {
                auto ptr_r = query.text.find_first_of("->", ptr_l+2);
                if (ptr_r != std::string::npos) {
                    route.push_back( std::string_view(query.text).substr(ptr_l+2,ptr_r-ptr_l-3) );
                    ptr_l = ptr_r;
                } else {
                    route.push_back( std::string_view(query.text).substr(ptr_l+2) );
                    break;
                }
            }
            if (query.text[ptr_l] == '-')
                for (int i = route.size()-2; i>=0 ; --i)
                    route.push_back(route[i]);

            catalogue_->AddBus(query.name, route);
        }
    }

    for (auto It = querys_.begin(); It->is_stop; ++It) {
        if (It->text.empty())
            continue;
        std::list<std::pair<std::string_view, size_t>> distance;

        size_t ptr_l = -2;
        for(;;) {
            auto ptr_m = It->text.find('m', ptr_l + 2);
            auto ptr_r = It->text.find(',', ptr_l + 2);
            if (ptr_r != std::string::npos) {
                distance.push_back( std::make_pair( std::string_view(It->text).substr(ptr_m+5,ptr_r-ptr_m-5),
                                                    std::stoul(It->text.substr(ptr_l+2, ptr_m-ptr_l))
                                                    )
                                    );
                ptr_l = ptr_r;
            } else {
                distance.push_back( std::make_pair( std::string_view(It->text).substr(ptr_m+5),
                                    std::stoul(It->text.substr(ptr_l+2, ptr_m-ptr_l))
                                                    )
                                    );
                break;
            }
        }
        catalogue_->AddDistance(It->name, distance);
    }
}
