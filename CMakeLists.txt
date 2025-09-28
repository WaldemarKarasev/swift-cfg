cmake_minimum_required(VERSION 3.16)
project(swift-cfg LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

# output paths setup
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# tree-sitter setup
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)
add_subdirectory(vendor/tree-sitter)

# tree-sitter-swift setup
include(cmake/AddTreeSitterSwift.cmake)

add_subdirectory(src)