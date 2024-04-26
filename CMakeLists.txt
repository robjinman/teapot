cmake_minimum_required(VERSION 3.27)

project(teapot CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB CPP_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(teapot ${CPP_SOURCES})

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  target_compile_options(teapot PRIVATE /W4 /O2)
else()
  target_compile_options(teapot PRIVATE -Wall -Wextra -O3)
endif()

file(COPY "${PROJECT_SOURCE_DIR}/teapot.obj" DESTINATION "${PROJECT_BINARY_DIR}")
