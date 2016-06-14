PROJECT(mrpc)

add_library(mrpc SHARED
    proxy
    node
    transport
    socket_transport
    message
    routing
    service
    path
    uuid
)
target_include_directories (mrpc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(mrpc jsoncpp_lib_static)

target_compile_options(mrpc PRIVATE -std=c++11 -pthread)