PROJECT(jrpc)

add_library(jrpc SHARED
    debug.c
    sockets.c
    jrpc_proxy
    jrpc_server.c
    message.c
)
target_include_directories (jrpc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(jrpc json jsoncpp_lib_static)