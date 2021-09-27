cmake_minimum_required(VERSION 3.16)

project(asio VERSION 0.0.1)

# This include defines standard install locations.
# https://cmake.org/cmake/help/v3.11/module/GNUInstallDirs.html
include(GNUInstallDirs)

# This include provides some helper functions that simplify packaging.
# https://cmake.org/cmake/help/v3.11/module/CMakePackageConfigHelpers.html
include(CMakePackageConfigHelpers)

add_subdirectory(./src)
add_subdirectory(./example)

# This will allow us to print a feature summary.
# https://cmake.org/cmake/help/v3.11/module/FeatureSummary.html
include(FeatureSummary)

# Print a summary of the features.
if(TARGET gnf::gnf)
  feature_summary(WHAT ALL)
endif(TARGET gnf::gnf)


