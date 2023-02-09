set(TARGET_NAME markovjunior)

set(${TARGET_NAME}_HEADERS
    AllNode.h
    Commons.h
    Constants.h
    ConvChain.h
    ConvolutionNode.h
    Field.h
    Grid.h
    GridUtils.h
    ImageHelpers.h
    Interpreter.h
    Map.h
    Node.h
    Observation.h
    OneNode.h
    ParallelNode.h
    Path.h
    Rule.h
    RuleNode.h
    Search.h
    SymmetryUtils.h
    WaveFunctionCollapse.h
)

set(${TARGET_NAME}_SOURCES
    AllNode.cpp
    Constants.cpp
    ConvChain.cpp
    ConvolutionNode.cpp
    Field.cpp
    Grid.cpp
    GridUtils.cpp
    ImageHelpers.cpp
    Interpreter.cpp
    Map.cpp
    Node.cpp
    Observation.cpp
    OneNode.cpp
    ParallelNode.cpp
    Path.cpp
    Rule.cpp
    RuleNode.cpp
    Search.cpp
    SymmetryUtils.cpp
    WaveFunctionCollapse.cpp
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}
             FILES ${${TARGET_NAME}_HEADERS} ${${TARGET_NAME}_SOURCES}
)

add_library(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS})

add_library(ad::${TARGET_NAME} ALIAS ${TARGET_NAME})

# Custom target to have the files show up in the IDE
add_custom_target(${TARGET_NAME}_IDE
    SOURCES
        ${${TARGET_NAME}_HEADERS}
        ${${TARGET_NAME}_SOURCES}
)

cmc_target_current_include_directory(${TARGET_NAME})

cmc_cpp_all_warnings_as_errors(${TARGET_NAME} ENABLED ${BUILD_CONF_WarningAsError})

cmc_cpp_sanitizer(${TARGET_NAME} ${BUILD_CONF_Sanitizer})


##
## Dependencies
##

cmc_find_dependencies(FILE CMakeFinds.cmake.in)


target_link_libraries(${TARGET_NAME}
    PUBLIC
        ad::platform
        ad::resource
        ad::math
        ad::handy
        ad::arte
        ad::imguiui
        pugixml::pugixml
)

##
## Install
##

install(TARGETS ${TARGET_NAME} EXPORT ${TARGET_NAME}Targets)

include(cmc-install)
# Install the header files
cmc_install_with_folders(DESTINATION include/${TARGET_NAME}/${TARGET_NAME}
                         FILES ${${TARGET_NAME}_HEADERS})
# Prepare CMake package in both build and install trees
cmc_install_packageconfig(${TARGET_NAME} ${TARGET_NAME}Targets ${PROJECT_NAME}
                          FIND_FILE CMakeFinds.cmake.in
                          NAMESPACE ad::)
