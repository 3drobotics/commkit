
find_library(LIBFASTRTPS fastrtps)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../shared
)

add_executable(pub_fastrtps
    test_pub_fastrtps.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../test_config.cpp
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

target_link_libraries(pub_fastrtps ${LIBFASTRTPS})
