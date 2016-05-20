
find_library(LIBFASTRTPS fastrtps)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../shared
)

add_executable(sub_rtps
    test_sub_rtps.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../time_util.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../test_config.cpp
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

target_link_libraries(sub_rtps ${LIBFASTRTPS})
