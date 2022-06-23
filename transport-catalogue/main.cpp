#include <iostream>


#include "request_handler.h"
#include "json_reader.h"

int main () {
    request_handler::RequestHandler handler;
    json_reader::JsonReader reader(&handler);

    reader.ReadJSON(std::cin);

    reader.WriteJSON(std::cout);

    return 0;
}
