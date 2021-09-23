
cmake_minimum_required(VERSION 3.16)

project(asio)

find_package(Boost REQUIRED COMPONENTS system thread)
find_package(fmt REQUIRED )

add_executable(server ./server.cpp)
target_link_libraries(server PUBLIC Boost::system Boost::thread fmt::fmt)
target_compile_features(server PRIVATE cxx_std_17 )

add_executable(client ./client.cpp)
target_link_libraries(client PUBLIC Boost::system Boost::thread fmt::fmt)
target_compile_features(server PRIVATE cxx_std_17 )
