cmake_minimum_required(VERSION 3.0.2)

project(libnekit C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_COMPILER_IS_GNUCC)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
  endif ()
else ()
  set (CMAKE_C_STANDARD 99)
endif ()

if (NOT NEKIT_ARCH)
  set(NEKIT_ARCH "x64")
endif()

if ("${NEKIT_ARCH}" STREQUAL "x32")
  message(STATUS "Compile in 32 bit.")
  if (UNIX AND NOT APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mx32")
    set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -mx32")
  elseif(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "i386")
  endif()
elseif("${NEKIT_ARCH}" STREQUAL "ia32")
  if (UNIX AND NOT APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -m32")
  elseif(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "i386")
  endif()
else()
  message(STATUS "Compile in 64 bit.")
  if (UNIX AND NOT APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
    set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -m64")
  elseif(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
  endif()
endif()

unset(CMAKE_OSX_DEPLOYMENT_TARGET)

include(CheckCCompilerFlag)

check_c_compiler_flag(-Wall HAS_WALL)
if(HAS_WALL)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif()

check_c_compiler_flag(-Wextra HAS_WEXTRA)
if(HAS_WEXTRA)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
endif()

check_c_compiler_flag(-pedantic HAS_PEDANTIC)
if(HAS_PEDANTIC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "COVERAGE")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set(CMAKE_LINK_LIBRARY_FLAG "${CMAKE_LINK_LIBRARY_FLAG} --coverage")
endif()

set(SOURCES
  src/utils/memory_pool.c
  # instance.c
  # server/tcp_server.c
  # socket/tcp_listen_socket.c
  src/socket/tcp_socket.c
  # src/socket/proxy/sock5_socket.c
  )

link_directories(deps/deps_build/lib)
include_directories(deps/deps_build/include)

include_directories(BEFORE include)

add_library(nekit ${SOURCES})
target_include_directories(nekit INTERFACE include)
find_library(UV_LIBRARY NAMES uv libuv HINTS deps/deps_build/lib)
set(LIBS ${LIBS} ${UV_LIBRARY})
if (WIN32)
  set(LIBS ${LIBS} ws2_32 iphlpapi psapi userenv)
  set(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"LIBCMTD.lib\")
endif()
target_link_libraries(nekit ${LIBS})

enable_testing()
add_subdirectory(test)
