## \brief Installs the provided files (optionally prepending DESTINATION prefix)
##        while preserving the relative path of each file (i.e. recreating folder hierarchy).
##
## \arg DESTINATION required path that will be prefixed to the provided FILES
## \arg FILES list of relative paths to files, prepended with DESTINATION when installed
function(cmc_install_with_folders)
    set(optionsArgs "")
    set(oneValueArgs "DESTINATION")
    set(multiValueArgs "FILES")
    cmake_parse_arguments(CAS "${optionsArgs}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    if (NOT CAS_DESTINATION)
        message(AUTHOR_WARNING "DESTINATION value is required, skip installation for FILES")
        return()
    endif()

    foreach(_file ${CAS_FILES})
        get_filename_component(_dir ${_file} DIRECTORY)
        install(FILES ${_file} DESTINATION ${CAS_DESTINATION}/${_dir})
    endforeach()
endfunction()


## \brief Internal helper function to generate config check files in both buid and install trees.
##
## \arg BUILDTREE_DESTINATION Path where the version config file will be created in build tree.
## \arg INSTALL_DESTINATION Path where the version config file will be installed.
function(_version_files PACKAGE_NAME BUILDTREE_DESTINATION INSTALL_DESTINATION
                        VERSION VERSION_COMPATIBILITY ARCH_INDEPENDENT)
    if (ARCH_INDEPENDENT)
        set(_ARCH "ARCH_INDEPENDENT")
    endif()
    # Build tree
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(${BUILDTREE_DESTINATION}/${PACKAGE_NAME}ConfigVersion.cmake
                                     VERSION ${VERSION}
                                     COMPATIBILITY ${VERSION_COMPATIBILITY}
                                     ${_ARCH})
    # Install tree
    install(FILES ${BUILDTREE_DESTINATION}/${PACKAGE_NAME}ConfigVersion.cmake
            DESTINATION ${INSTALL_DESTINATION})
endfunction()


## \brief Generate a package config file for TARGET, making it available:
##        * (at configure time) in the build tree
##        * (at install time) in the install tree
##
## \arg TARGET Name of the target that will become a component of the package.
## \arg EXPORTNAME Name of the export set for which targets files are generated.
## \arg PACKAGE_NAME Name of the package of which TARGET is made a component.
## \arg FIND_FILE optional find file templates (see cmc_find_dependencies for syntax), invoked
##      by the generated Config file for TARGET. Allows finding external dependencies, while
##      keeping the list of said external dependencies DRY (thanks to the template re-use).
## \arg VERSION_COMPATIBILITY optional version compatibility mode for the created package.
##      Accepted values are the values for COMPATIBILITY of write_basic_package_version_file:
##      https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html#generating-a-package-version-file
##      When absent, the package version file will **not** be produced, so no version checks are made.
##      When provided, the VERSION property of TARGET will be used as package version.
## \arg ARCH_INDEPENDENT optional flag, whose presence make the package version compatible accross
##      different architectures.
##      see: https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html#command:write_basic_package_version_file
##      This flag will only have an effect if the version file is produced.
##      i.e. it should only be provided when VERSION_COMPATIBILITY is also provided.
## \arg NAMESPACE Prepended to all targets written in the export set.
function(cmc_install_packageconfig TARGET EXPORTNAME PACKAGE_NAME)
    set(optionsArgs "ARCH_INDEPENDENT")
    set(oneValueArgs "NAMESPACE" "FIND_FILE" "VERSION_COMPATIBILITY")
    set(multiValueArgs "")
    cmake_parse_arguments(CAS "${optionsArgs}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    set(_install_destination ${CMC_INSTALL_CONFIGPACKAGE_PREFIX}/${PACKAGE_NAME}/${TARGET})
    # Note: using PROJECT_BINARY_DIR relies on the convention that the targets
    # become components for the repository's only project (in the repositoryes root CMakeLists).
    # It will not necessarily be the top-level project of the build tree (e.g. Conan Workspaces).
    set(_buildtree_destination ${PROJECT_BINARY_DIR}/${TARGET})

    # If a find file is provided to find upstreams
    set(_findupstream_file ${TARGET}FindUpstream.cmake)
    if (CAS_FIND_FILE)
        # No value for REQUIRED and QUIET substition, to remove them
        set(find_package "find_dependency")
        # build tree
        configure_file(${CAS_FIND_FILE} ${_buildtree_destination}/${_findupstream_file} @ONLY)
        #install tree
        install(FILES ${_buildtree_destination}/${_findupstream_file}
                DESTINATION ${_install_destination})
    endif()

    set(_targetfile "${EXPORTNAME}.cmake")

    # Generate config files in the build tree
    configure_file(${CMC_ROOT_DIR}/templates/PackageConfig.cmake.in
                   ${_buildtree_destination}/${TARGET}Config.cmake
                   @ONLY)

    # Install the config file over to the install tree
    install(FILES ${_buildtree_destination}/${TARGET}Config.cmake
            DESTINATION ${_install_destination})

    # build tree
    export(EXPORT ${EXPORTNAME}
        FILE ${_buildtree_destination}/${_targetfile}
        NAMESPACE ${CAS_NAMESPACE})

    # install tree
    install(EXPORT ${EXPORTNAME}
        FILE ${_targetfile}
        DESTINATION ${_install_destination}
        NAMESPACE ${CAS_NAMESPACE})

    #
    # Optional version logic
    #
    if (CAS_VERSION_COMPATIBILITY)
        get_target_property(_version ${TARGET} VERSION)
        if(NOT _version)
            message(SEND_ERROR "VERSION property must be set on target ${TARGET} before setting its VERSION_COMPATIBILITY")
        endif()
        _version_files(${TARGET} ${_buildtree_destination} ${_install_destination}
                       ${_version} ${CAS_VERSION_COMPATIBILITY} ${CAS_ARCH_INDEPENDENT})
    endif()
endfunction()


## \brief Generate the root package config file for a project providing several components
##        (typically, a repository containing several libraries).
##
## \arg PACKAGE_NAME the name of the created package.
## \arg VERSION_COMPATIBILITY optional version compatibility mode for the created package.
##      Accepted values are the values for COMPATIBILITY of write_basic_package_version_file:
##      https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html#generating-a-package-version-file
##      When absent, the package version file will **not** be produced, so no version checks are made.
##      When provided, CMAKE_PROJECT_VERSION will be used as package version.
## \arg ARCH_INDEPENDENT optional flag, whose presence make the package version compatible accross
##      different architectures.
##      see: https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html#command:write_basic_package_version_file
##      This flag will only have an effect if the version file is produced.
##      i.e. it should only be provided when VERSION_COMPATIBILITY is also provided.
##
## This config file should be found by downstream in its call to
## find_package(${PACKAGE_NAME} COMPONENTS ...).
function(cmc_install_root_component_config PACKAGE_NAME)
    set(optionsArgs "ARCH_INDEPENDENT")
    set(oneValueArgs "VERSION_COMPATIBILITY")
    cmake_parse_arguments(CAS "${optionsArgs}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    # Generate root config files in the build tree
    configure_file(${CMC_ROOT_DIR}/templates/ComponentPackageRootConfig.cmake.in
                   ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}Config.cmake
                   @ONLY)

    # Install the root config file over to the install tree
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}Config.cmake
            DESTINATION ${CMC_INSTALL_CONFIGPACKAGE_PREFIX}/${PACKAGE_NAME})

    if (CAS_VERSION_COMPATIBILITY)
        if(NOT CMAKE_PROJECT_VERSION)
            message(SEND_ERROR "Top level CMake project must have a version set before setting VERSION_COMPATIBILITY")
        endif()
        _version_files(${PACKAGE_NAME}
                       ${CMAKE_CURRENT_BINARY_DIR}
                       ${CMC_INSTALL_CONFIGPACKAGE_PREFIX}/${PACKAGE_NAME}
                       ${CMAKE_PROJECT_VERSION}
                       ${CAS_VERSION_COMPATIBILITY}
                       ${CAS_ARCH_INDEPENDENT})
    endif()
endfunction()


## \brief Disables any attempt to find PACKAGE_NAME package via find_package() calls.
## Relies on the CONFIG mode.
##
## \arg PACKAGE_NAME the name of the CMake package to disable.
##
## This is usefull to allow downstreams to treat all dependencies the same:
## locate them with find_package(), then use them via target_link_libraries().
## It should not matter whether the dependencies is part of the current build tree
## or not. (This allows advanced generic usages, such as Conan Workspaces.).
## General discussion: https://gitlab.kitware.com/cmake/cmake/-/issues/17735#note_487572
##
## The name was chosen following Daniel Pfeifer note.
## see: https://gitlab.kitware.com/cmake/cmake/-/issues/17735#note_487572
function(cmc_register_source_package PACKAGE_NAME)
  file(WRITE "${CMAKE_BINARY_DIR}/__pkg/${PACKAGE_NAME}/${PACKAGE_NAME}Config.cmake"
       "message(VERBOSE \"find_package(${PACKAGE_NAME}) disabled: package targets are directly available in the build tree.\")")
  set(${PACKAGE_NAME}_DIR ${CMAKE_BINARY_DIR}/__pkg/${PACKAGE_NAME} CACHE PATH "")
endfunction()
