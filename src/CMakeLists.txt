PROJECT(jrpc)

add_library(jrpc SHARED
    proxy
    node
    transport
    socket_transport
)
target_include_directories (jrpc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(jrpc jsoncpp_lib_static)

target_compile_options(jrpc PRIVATE -std=c++11 -pthread)