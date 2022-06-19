#include <iostream>


#include "request_handler.h"
#include "json_reader.h"

int main () {
    request_handler::RequestHandler handler;
    json_reader::JsonReader reader(&handler);


//    std::string str;
//    getline(std::cin, str);
//    int n = std::stoi(str);
//
//    transport_catalogue::TransportCatalogue transport_catalogue;
//
//    for ( transport_catalogue::InputReader input(&transport_catalogue); n; --n) {
//        getline(std::cin, str);
//        input.AddQuery (std::move(str));
//    }
//
//    getline(std::cin, str);
//    n = std::stoi(str);
//
//    for (transport_catalogue::StatReader output(&transport_catalogue); n; --n) {
//        getline(std::cin, str);
//        std::cout<<output.GetStat(str);
//    }

    return 0;
}
