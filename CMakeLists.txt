cmake_minimum_required(VERSION 3.7.1)
project(libnekit VERSION 0.0.1 LANGUAGES C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_library(nekit SHARED utils/linked_list.c)
