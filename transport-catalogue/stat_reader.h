#pragma once

#include <string>
#include <vector>
#include <optional>

#include "transport_catalogue.h"

//struct BusDate {
//    bool is_correct = false;
//    unsigned int stops;
//    unsigned int unique_stops;
//    double length;
//
//};

class StatReader {

TransportCatalogue* const catalogue_;

public:
    explicit StatReader (TransportCatalogue* const trans_cat) : catalogue_(trans_cat) {
    }

    void GetStat(std::string_view str) {
        //std::literals::string_literals::operator""s;

        auto n = str.find(' ')+1;

        std::string_view tipe = str.substr(0,n);
        str.remove_prefix(n);
//        std::cout<<str<<'|'<<std::endl;
        try{
            if (tipe == "Bus ") {
                auto tmp = catalogue_->GetBusInfo(str);
                std::cout<<tipe<<str<<": "<<tmp.stops<<" stops on route, "<<tmp.unique_stops<<
                " unique stops, "<<tmp.length<<" route length, "<<std::setprecision(6)<<tmp.curvature<<" curvature\n";
            } else {
                const auto buses = catalogue_->GetStopInfo(str);
                std::cout<<tipe<<str;
                std::cout<<(buses.empty()? ": no buses" : ": buses ");

                for (const auto& bus : buses)
                    std::cout<<bus<<' ';
                std::cout<<'\n';
            }

        }
        catch (.../*std::out_of_range exc*/) {
            std::cout<<tipe<<str<<": not found\n";
        }
    }

};
