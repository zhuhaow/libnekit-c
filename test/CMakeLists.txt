enable_testing()

set(CHECK_ENABLE_TESTS OFF)
add_subdirectory(deps/check)
set(CHECK_HEADER_PATH ${CMAKE_CURRENT_BINARY_DIR}/deps/check)
include_directories(${CHECK_HEADER_PATH} ${CHECK_HEADER_PATH}/src)

set(LIBS compat check nekit)
include_directories(../include)

find_program(MEMORYCHECK_COMMAND valgrind)
set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full")

add_executable(test_memory_pool utils/test_memory_pool.c)
target_link_libraries(test_memory_pool ${LIBS})
add_test(test_memory_pool ${CMAKE_CURRENT_BINARY_DIR}/test_memory_pool)



