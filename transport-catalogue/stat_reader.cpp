#include "stat_reader.h"

#include <iostream>
#include <optional>
#include <iomanip>

using namespace transport_catalogue;

void  StatReader::GetStat(std::string_view str) const {
    using std::literals::string_literals::operator""s;

    auto n = str.find(' ')+1;

    std::string_view tipe = str.substr(0,n);
    str.remove_prefix(n);
    try{
        if (tipe == "Bus "s) {
            auto tmp = catalogue_->GetBusInfo(str);
            std::cout<<tipe<<str<<": "s<<tmp.stops<<" stops on route, "s<<tmp.unique_stops<<
            " unique stops, "s<<tmp.length<<" route length, "s<<std::setprecision(6)<<tmp.curvature<<" curvature\n"s;
        } else {
            const auto buses = catalogue_ -> GetStopInfo(str);
            std::cout<<tipe<<str;
            std::cout<<(buses.empty()? ": no buses"s : ": buses "s);

            for (const auto& bus : buses)
                std::cout<<bus<<' ';
            std::cout<<'\n';
        }

    }
    catch (const std::out_of_range& exp) {
        std::cout<<tipe<<str<<": not found\n"s;
    }
}
