#include "stat_reader.h"

#include <iostream>
#include <optional>
#include <iomanip>

#include <sstream>

using namespace transport_catalogue;

std::string  StatReader::GetStat(std::string_view str) const {
    using std::literals::string_literals::operator""s;

    auto n = str.find(' ')+1;

    std::string_view tipe = str.substr(0,n);
    str.remove_prefix(n);
    std::stringstream ans;
    try{
        if (tipe == "Bus "s) {
            auto tmp = catalogue_->GetBusInfo(str);
            ans<<tipe<<str<<": "s<<tmp.stops<<" stops on route, "s<<tmp.unique_stops<<
            " unique stops, "s<<tmp.length<<" route length, "s<<std::setprecision(6)<<tmp.curvature<<" curvature\n"s;
        } else {
            const auto buses = catalogue_ -> GetStopInfo(str);
            ans<<tipe<<str
            <<(buses.empty()? ": no buses"s : ": buses "s);

            for (const auto& bus : buses)
                ans<<bus<<' ';
            ans<<'\n';
        }

    }
    catch (const std::out_of_range& exp) {
        ans<<tipe<<str<<": not found\n"s;
    }
    return ans.str();
}
