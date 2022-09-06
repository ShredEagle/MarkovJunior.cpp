## \brief Enable all warnings as error for provided TARGET.
##        Additionally raises the warning level except for MSVC.
function(cmc_cpp_all_warnings_as_errors TARGET)

    set(gnuoptions "AppleClang" "Clang" "GNU")
    if (CMAKE_CXX_COMPILER_ID IN_LIST gnuoptions)
        set(option "-Werror")
        # Enables all warnings
        # Except on MSVC, where all warnings create too many problems with system headers
        target_compile_options(${TARGET} PRIVATE -Wall)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(option "-WX")
    endif()

    # The option is given as both compile option and link option
    # Note:: target_link_options is only available from 3.13
    target_compile_options(${TARGET} PRIVATE ${option})
    target_link_libraries(${TARGET} PRIVATE ${option})

endfunction()
