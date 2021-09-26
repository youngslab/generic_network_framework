cmake_minimum_required(VERSION 3.16)

project(asio VERSION 0.0.1)

# This include defines standard install locations.
# https://cmake.org/cmake/help/v3.11/module/GNUInstallDirs.html
include(GNUInstallDirs)

# This include provides some helper functions that simplify packaging.
# https://cmake.org/cmake/help/v3.11/module/CMakePackageConfigHelpers.html
include(CMakePackageConfigHelpers)

find_package(Boost QUIET REQUIRED COMPONENTS system thread)
find_package(fmt REQUIRED)

add_library(gnf INTERFACE)
target_include_directories(gnf INTERFACE ./include)

target_include_directories(
  gnf
  INTERFACE # Used when building the library:
            $<BUILD_INTERFACE:${gnf_SOURCE_DIR}/include>
            # Used when installing the library:
            $<INSTALL_INTERFACE:include/${PROJECT_PREFIX}>)

target_link_libraries(gnf INTERFACE fmt::fmt Boost::thread Boost::system)

add_library(gnf::gnf ALIAS gnf)

# This will allow us to print a feature summary.
# https://cmake.org/cmake/help/v3.11/module/FeatureSummary.html
include(FeatureSummary)

# Print a summary of the features.
if(TARGET gnf::gnf)
  feature_summary(WHAT ALL)
endif(TARGET gnf::gnf)

