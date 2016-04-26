
# defined in gtest.cmake
link_directories(${GTEST_LIB_DIR})
include_directories(${GTEST_INCLUDE_DIR})

find_package(Threads REQUIRED)

set(TEST_SOURCES
    main.cpp
    basics.cpp
    chronoimpl.cpp
)

add_executable(commkit-tests ${TEST_SOURCES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

target_link_libraries(commkit-tests commkit_shared googletest ${CMAKE_THREAD_LIBS_INIT})
