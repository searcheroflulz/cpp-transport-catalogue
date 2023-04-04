cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 20)

set(Protobuf_PREFIX_PATH
        "D:/path_19_5/to/protobuf/package/include"
        "D:/path_19_5/to/protobuf/package/lib"
        "D:/path_19_5/to/protobuf/package/bin" )
list(APPEND CMAKE_PREFIX_PATH "${Protobuf_PREFIX_PATH}")

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(TRANSPORT_CATALOUGE_PROTO_SRCS TRANSPORT_CATALOUGE_PROTO_HDRS transport_catalogue.proto)
protobuf_generate_cpp(SVG_PROTO_SRCS SVG_PROTO_HDRS svg.proto)
protobuf_generate_cpp(MAP_RENDERER_PROTO_SRCS MAP_RENDERER_PROTO_HDRS map_renderer.proto)
protobuf_generate_cpp(TRANSPOR_ROUTER_PROTO_SRCS MAP_RENDERER_PROTO_HDRS transport_router.proto)
protobuf_generate_cpp(GRAPH_PROTO_SRCS MAP_RENDERER_PROTO_HDRS graph.proto)

set(MAIN main.cpp)
set(GEO_FILES geo.h)
set(TRANSPORT_CATALOUGE_FILES transport_catalogue.h transport_catalogue.cpp)
set(TRANSPOR_ROUTER_FILES ranges.h router.h graph.h transport_router.h transport_router.cpp)
set(JSON_FILES json.h json.cpp json_builder.h json_builder.cpp json_reader.h json_reader.cpp)
set(SVG_FILES svg.h svg.cpp)
set(MAP_RENDERER_FILES map_renderer.h map_renderer.cpp)
set(SERIALIZATION_FILES serialization.h serialization.cpp)
set(DOMAIN_FILE domain.h)

add_executable(transport_catalogue  ${MAIN} ${GEO_FILES} ${TRANSPORT_CATALOUGE_FILES}
        ${TRANSPOR_ROUTER_FILES} ${JSON_FILES} ${SVG_FILES}
        ${MAP_RENDERER_FILES} ${SERIALIZATION_FILES} ${DOMAIN_FILE} ${TRANSPORT_CATALOUGE_PROTO_SRCS}
        ${TRANSPORT_CATALOUGE_PROTO_HDRS} ${SVG_PROTO_SRCS} ${SVG_PROTO_HDRS}
        ${MAP_RENDERER_PROTO_SRCS} ${MAP_RENDERER_PROTO_HDRS}
        ${TRANSPOR_ROUTER_PROTO_SRCS} ${MAP_RENDERER_PROTO_HDRS}
        ${GRAPH_PROTO_SRCS} ${MAP_RENDERER_PROTO_HDRS})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)