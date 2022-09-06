add_subdirectory(libs/markovjunior/markovjunior)
add_subdirectory(apps/markov-proto/markov-proto)

option(BUILD_tests "Build the test applications" ON)
if(BUILD_tests)
endif()
