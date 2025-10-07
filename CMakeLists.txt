cmake_minimum_required(VERSION 3.16)
project(swift-cfg LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

# output paths setup
set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include(FetchContent)
# JSON 
FetchContent_Declare(
	json 
	URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz
)
FetchContent_MakeAvailable(json)


# tree-sitter setup
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)
add_subdirectory(vendor/tree-sitter)

# tree-sitter-swift setup
include(cmake/AddTreeSitterSwift.cmake)

# move code examples into bin directory
file(COPY code_examples DESTINATION ${OUTPUT_DIR})

add_subdirectory(src)