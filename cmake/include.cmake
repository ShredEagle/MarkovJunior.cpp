set (CMC_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

# Where to look for the files defined by this project
list(APPEND CMAKE_MODULE_PATH 
     "${CMC_ROOT_DIR}/templates"
     "${CMC_ROOT_DIR}/utilities")

# Functions in base are made available by default
# other files in utilities/ should be included when needed
include(cmc-base)

# Custom variables allowing customizatin of cmc utilities default behaviour
set(CMC_INSTALL_CONFIGPACKAGE_PREFIX "lib/cmake" CACHE STRING
    "Prefix for CMake package config in the install tree")
mark_as_advanced(CMC_INSTALL_CONFIGPACKAGE_PREFIX)
