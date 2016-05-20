
# defined in gtest.cmake
link_directories(${GTEST_LIB_DIR})

include_directories(
    ${GTEST_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
)

find_package(Threads REQUIRED)

find_library(LIBFASTRTPS fastrtps HINTS ${FASTRTPS}/lib)

set(TEST_SOURCES
    main.cpp
    basics.cpp
)

add_executable(fastrtps-tests ${TEST_SOURCES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0")

target_link_libraries(fastrtps-tests ${LIBFASTRTPS} googletest ${CMAKE_THREAD_LIBS_INIT})
