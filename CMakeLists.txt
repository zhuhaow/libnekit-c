cmake_minimum_required(VERSION 3.7.1)

project(libnekit VERSION 0.0.1 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(src
  src/utils/linked_list.c
  src/utils/memory_pool.c)

add_subdirectory(deps/libuv)

add_library(nekit ${src})
target_link_libraries(nekit uv)
