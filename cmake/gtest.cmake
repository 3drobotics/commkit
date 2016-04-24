
include(ExternalProject)
ExternalProject_Add(googletest-download
  # GIT_REPOSITORY    https://github.com/google/googletest.git
  # GIT_TAG           master
  URL               https://github.com/google/googletest/archive/d225acc90bc3a8c420a9bcd1f033033c1ccd7fe0.zip
  URL_MD5           ef6a52cb3966c21cef475b53f4677486
  SOURCE_DIR        ${CMAKE_BINARY_DIR}/googletest-src
  BINARY_DIR        ${CMAKE_BINARY_DIR}/googletest-build
  CMAKE_ARGS        -DBUILD_GMOCK=OFF -DBUILD_GTEST=ON
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)

SET(GTEST_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/googletest-src/googletest/include)
SET(GTEST_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/googletest-build/googletest)

# for other targets to depend on
add_library(googletest STATIC IMPORTED GLOBAL)
add_dependencies(googletest googletest-download)
set_target_properties(googletest PROPERTIES IMPORTED_LOCATION
    ${GTEST_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX})
