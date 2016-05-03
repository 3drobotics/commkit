
find_library(LIBFASTRTPS fastrtps)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(pub_commkit
    test_pub_commkit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../test_config.cpp
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

target_link_libraries(pub_commkit commkit_shared ${LIBFASTRTPS})
