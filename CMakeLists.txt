cmake_minimum_required(VERSION 3.16)

project(MarkovJunior VERSION "${BUILD_VERSION}")

include(cmake/include.cmake)

cmc_include_conan_configuration()

# Enable the grouping target in folders, when available in IDE.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
#    set(LINK_OPTIONS "/INCREMENTAL:NO")
#endif()

# Install the top-level package config, allowing to find all sub-components
include(cmc-install)
cmc_install_root_component_config(${PROJECT_NAME})
cmc_register_source_package(${PROJECT_NAME})

add_subdirectory(src)
