#include <fstream>
#include <iostream>
#include <string_view>

#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "bus_router.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    request_handler::RequestHandler handler;
    renderer::MapRenderer render;
    router::BusRouter router;
    serialization::Serializator serializator(&handler, &render, &router);
    json_reader::JsonReader reader(&handler, &render, &router, &serializator);

    if (mode == "make_base"sv) {

        reader.ReadJSON(std::cin);
        serializator.SaveData();

    } else if (mode == "process_requests"sv) {

        reader.ReadJSON(std::cin);
        serializator.LoadData();
        reader.WriteJSON(std::cout);

    } else {
        PrintUsage();
        return 1;
    }

    return 0;
}
