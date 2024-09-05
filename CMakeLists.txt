cmake_minimum_required(VERSION 3.24)

project(str-to-num
  LANGUAGES CXX
)

#set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COMPILE_WARNING_AS_ERROR True)
set(CMAKE_ERROR_DEPRECATED True)

enable_testing()

file(GLOB_RECURSE test_files CONFIGURE_DEPENDS
  str-to-num.hpp
  test/*.cpp
  test/*.hpp
)
message("files: ${test_files}")
add_executable(str-to-num
  "${test_files}"
)
target_include_directories(str-to-num
  PUBLIC "${CMAKE_SOURCE_DIR}"
)
# https://gitlab.kitware.com/cmake/cmake/-/issues/18837#note_722441
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
	target_compile_options(str-to-num PUBLIC "/Zc:__cplusplus")
endif()

add_test(
  NAME test-str-to-num
  COMMAND str-to-num
)
