cmake_minimum_required(VERSION 3.10)

project(CppLinuxSocketsApp LANGUAGES C CXX)

add_subdirectory(net)

include_directories(include)

add_executable(server main_server.cc interrupt.cc processor.cc)
target_link_libraries(server PRIVATE net)

add_executable(client main_client.cc interrupt.cc processor.cc)
target_link_libraries(client PRIVATE net)
