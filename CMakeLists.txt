cmake_minimum_required(VERSION 2.8.12.2)

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

if ("${NEKIT_ARCH}" STREQUAL "x86")
  message(STATUS "Compile in 32 bit.")
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

function(enable_c_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_C_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_c_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
    endif()
endfunction()

enable_c_compiler_flag_if_supported("-Wall")
enable_c_compiler_flag_if_supported("-Wextra")
enable_c_compiler_flag_if_supported("-pedantic")

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
