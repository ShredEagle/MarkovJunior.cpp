# CMake helper

CMake scripts to provide helpers and factorize common setups for CMake based projects.

## Usage

This project should be added as a submodule to downstream project repository,
cloning it in 'cmake/' subfolder:

    cd $PROJECT
    git submodule add ${repo_url} cmake


It is then a matter of including the `include.cmake` file at the beginning 
of downstream's root CMakeLists.txt

    cmake_minimum_required(VERSION 3.12)
    project(Downstream)

    # Include cmake_helper submodule
    include("cmake/include.cmake")

    ...


This will initialize cmake helpers, and make functions in `utilities/cmc-base.cmake` available.
Additional functions defined in other files from `utilities/` are made available
by including the corresponding file with its basename.
E.g. to access `cmc_install_with_folders` defined in `utilities/cmc-install.cmake`:

    ...

    include(cmc-install)
    cmc_install_with_folders(FILES a/a.h b/b.h DESTINATION include/)

    ...
