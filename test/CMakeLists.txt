enable_testing()

set(CHECK_ENABLE_TESTS OFF CACHE BOOL "Disable test for check." FORCE)
add_subdirectory(deps/check)
set(CHECK_HEADER_PATH ${CMAKE_CURRENT_BINARY_DIR}/deps/check)
include_directories(${CHECK_HEADER_PATH} ${CHECK_HEADER_PATH}/src)

set(LIBS compat check nekit)
include_directories(../include)

option(NE_ENABLE_MEMCHECK "Enable valgrind when running tests." ON)
find_program(CTEST_MEMORYCHECK_COMMAND valgrind)
set(CTEST_MEMORYCHECK_COMMAND_OPTIONS --trace-children=yes --leak-check=full --show-leak-kinds=all)

function(add_mem_test name)
  if (CTEST_MEMORYCHECK_COMMAND AND NE_ENABLE_MEMCHECK)
    message(STATUS "Adding valgrind test for ${name}.")
    set(MEMCHECK_PARAMS ${CTEST_MEMORYCHECK_COMMAND} ${CTEST_MEMORYCHECK_COMMAND_OPTIONS} $<TARGET_FILE:${name}>)
    add_test(NAME ${name} COMMAND ${MEMCHECK_PARAMS})
  else()
    message(STATUS "Adding test for ${name}.")
    add_test(${name} ${name})
  endif()
endfunction()

add_executable(test_memory_pool utils/test_memory_pool.c)
target_link_libraries(test_memory_pool ${LIBS})
add_mem_test(test_memory_pool)
