#include <iostream>


#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "bus_router.h"

int main () {
    request_handler::RequestHandler handler;
    renderer::MapRenderer render;
    router::BusRouter router;
    json_reader::JsonReader reader(&handler, &render, &router);

    reader.ReadJSON(std::cin);

    reader.WriteJSON(std::cout);

    return 0;
}
