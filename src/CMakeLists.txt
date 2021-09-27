
find_package(Boost QUIET REQUIRED COMPONENTS system thread)
find_package(fmt REQUIRED)

add_library(gnf INTERFACE)
target_include_directories(gnf INTERFACE ../include)

target_include_directories(
  gnf
  INTERFACE # Used when building the library:
            $<BUILD_INTERFACE:${gnf_SOURCE_DIR}/../include>
            # Used when installing the library:
            $<INSTALL_INTERFACE:include/${PROJECT_PREFIX}>)

target_link_libraries(gnf INTERFACE fmt::fmt Boost::thread Boost::system)

add_library(gnf::gnf ALIAS gnf)

