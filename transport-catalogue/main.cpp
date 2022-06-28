#include <iostream>


#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"

int main () {
    request_handler::RequestHandler handler;
    renderer::MapRenderer render(&handler);
    json_reader::JsonReader reader(&handler, &render);

    reader.ReadJSON(std::cin);

    //reader.WriteJSON(std::cout);

    return 0;
}
