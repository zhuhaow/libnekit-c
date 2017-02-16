cmake_minimum_required(VERSION 2.8.12)

project(libnekit C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
  src/utils/memory_pool.c
  # instance.c
  # server/tcp_server.c
  # socket/tcp_listen_socket.c
  src/socket/tcp_socket.c
  # src/socket/proxy/sock5_socket.c
  )

add_subdirectory(deps/libuv)
include_directories(deps/libuv/libuv/include)

include_directories(BEFORE include)

add_library(nekit ${SOURCES})
target_include_directories(nekit INTERFACE include)
target_link_libraries(nekit uv)
