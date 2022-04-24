#include <iostream>

#include <iomanip>

#include <string>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

int main () {

    std::string str;
    getline(std::cin, str);
    int n = std::stoi(str);

    TransportCatalogue transport_catalogue;

    for ( InputReader input(&transport_catalogue); n; --n) {
        getline(std::cin, str);
        input.AddQuery (std::move(str));
    }

    getline(std::cin, str);
    n = std::stoi(str);

    for (StatReader output(&transport_catalogue); n; --n) {
        getline(std::cin, str);
        output.GetStat(str);
    }



    return 0;
}
