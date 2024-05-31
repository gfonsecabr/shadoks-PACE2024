cmake_minimum_required(VERSION 3.5)
project(shadoks)

set(CMAKE_CXX_STANDARD 20)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "RELEASE")
ENDIF()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native -mtune=native -funroll-loops -finline-functions")

include_directories(".")

file(GLOB SOURCES "main.cpp")

add_executable(heuristic ${SOURCES} )

add_executable(exact ${SOURCES} )
target_compile_definitions(exact PRIVATE EXACT)
