
add_executable(test_rpc
    test_rpc
)

target_compile_options(test_rpc PRIVATE -std=c++11 -pthread)

target_link_libraries(test_rpc jrpc pthread)
