#pragma once

#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>

#include "domain.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "bus_router.h"
#include <transport_catalogue.pb.h>
#include <svg.pb.h>
#include <map_renderer.pb.h>
//#include <graph.pb.h>
#include <bus_router.pb.h>

namespace serialization {

using Path = std::filesystem::path;

class Serializator {

t_catalogue::TransportCatalogue* const catalogue_;
renderer::MapRenderer* const render_;
router::BusRouter* const router_;
Path path_to_bd_;

db::RouterSettings SerializationRoutSettings () const;

db::MapSettings SerializationMapSettings () const;

public:

    Serializator(request_handler::RequestHandler* const handler, renderer::MapRenderer* const render, router::BusRouter* const router) :
    catalogue_(handler->GetTransportCatalogue ()),
    render_(render),
    router_(router) {
    };

    Serializator& SetPath(const Path& path);

    void SaveData ();

    void LoadData ();

};
}
