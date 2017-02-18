cmake_minimum_required(VERSION 2.8.12)

project(libnekit C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_COMPILER_IS_GNUCC)
    set (CMAKE_C_FLAGS --std=gnu99 ${CMAKE_C_FLAGS})
  endif ()
else ()
  set (CMAKE_C_STANDARD 99)
endif ()

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
include_directories(dev_header)

add_library(nekit ${SOURCES})
target_include_directories(nekit INTERFACE include)
set(LIBS ${LIBS} uv)
target_link_libraries(nekit ${LIBS})

enable_testing()
add_subdirectory(test)
