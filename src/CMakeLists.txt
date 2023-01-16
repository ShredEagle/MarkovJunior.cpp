add_subdirectory(libs/markovjunior/markovjunior)

option(BUILD_tests "Build the test applications" ON)
if(BUILD_tests)
    add_subdirectory(apps/markov-proto/markov-proto)
endif()
