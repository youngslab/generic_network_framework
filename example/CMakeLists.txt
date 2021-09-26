# find_package(gnf CONFIG REQUIRED)

add_executable(server ./chat_server.cpp)
target_link_libraries(server PUBLIC gnf::gnf)

add_executable(client ./chat_client.cpp)
target_link_libraries(client PUBLIC gnf::gnf)
