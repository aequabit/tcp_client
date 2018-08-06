cmake_minimum_required(VERSION 3.9)
project(tcp_client)

set(CMAKE_CXX_STANDARD 17)

link_libraries(pthread)

add_executable(tcp_client src/main.cc src/tcp/client.cc)
