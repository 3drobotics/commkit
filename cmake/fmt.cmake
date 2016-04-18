
####################################
#
# add targets 'fmt' and 'fmt-diff'
#
####################################

find_package(PythonInterp)

# apply formatting
add_custom_target(fmt
    COMMAND ${PYTHON_EXECUTABLE} tools/clang-format-run.py --apply
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    SOURCES ${CMAKE_SOURCE_DIR}/.clang-format)

# verify formatting
add_custom_target(fmt-diff
    COMMAND ${PYTHON_EXECUTABLE} tools/clang-format-run.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    SOURCES ${CMAKE_SOURCE_DIR}/.clang-format)
