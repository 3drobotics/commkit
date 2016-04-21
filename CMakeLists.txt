
cmake_minimum_required(VERSION 2.8.12)

project(commkit)

# default MACOSX_RPATH to ON
if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

# path to compiled Fast-RTPS library must be provided
# XXX: contribute a FindFastRTPS.cmake upstream such that we can use find_package()
if (NOT FASTRTPS)
    message(FATAL_ERROR "error - FASTRTPS installation location not specified")
endif()
set(FASTRTPS_LIB ${FASTRTPS}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}fastrtps${CMAKE_SHARED_LIBRARY_SUFFIX})

# set default build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Select build type" FORCE)
endif()

set(COMMKIT_SRCS
    src/node.cpp
    src/nodeimpl.cpp
    src/publisher.cpp
    src/publisherimpl.cpp
    src/subscriber.cpp
    src/subscriberimpl.cpp
)

set(CMAKE_POSITION_INDEPENDENT_CODE True)

# build an OBJECT library, which is then used to generate both shared
# and static libs. this avoids specifying compile options (and compiling) twice.
add_library(commkit_obj OBJECT ${COMMKIT_SRCS})

if(${CMAKE_VERSION} VERSION_LESS "3.1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    # beware: https://cmake.org/Bug/view.php?id=15797
    set_target_properties(commkit_obj PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
    )
endif()

add_library(commkit_shared SHARED $<TARGET_OBJECTS:commkit_obj>)
add_library(commkit_static STATIC $<TARGET_OBJECTS:commkit_obj>)
set_target_properties(commkit_shared PROPERTIES OUTPUT_NAME commkit)
set_target_properties(commkit_static PROPERTIES OUTPUT_NAME commkit)

# ensure public API is exported, see visibility.h
add_definitions(-DCOMMKIT_DLL -DCOMMKIT_DLL_EXPORTS)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${FASTRTPS}/include
)

# dependencies - always rely on fast-rtps, optionally disable capnproto support

set(commkit_LIBS ${FASTRTPS_LIB})

if(COMMKIT_NO_CAPNP)
    add_definitions(-DCOMMKIT_NO_CAPNP)
else()
    # import manually until https://github.com/sandstorm-io/capnproto/pull/178 is resolved
    set(commkit_LIBS ${commkit_LIBS}
        ${CMAKE_SHARED_LIBRARY_PREFIX}capnp${CMAKE_SHARED_LIBRARY_SUFFIX}
        ${CMAKE_SHARED_LIBRARY_PREFIX}kj${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

target_link_libraries(commkit_shared ${commkit_LIBS})

# code formatting support
include(cmake/fmt.cmake)


# XXX: would like INCLUDE installation items to be determined via a target_include_directories()
# invocation, but those includes must be specified on the target being compiled, which in our case
# is an OBJECT library (commkit_obj) and cmake complains that it can't be installed:
#   "install TARGETS given OBJECT library "commkit_obj" which may not be installed"
# installing manually for now.
install(DIRECTORY include/commkit DESTINATION include)

install(TARGETS commkit_shared commkit_static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
