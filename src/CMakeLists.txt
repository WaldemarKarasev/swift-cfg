cmake_minimum_required(VERSION 3.16)
project(swift-cfg LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)


add_executable(swift_cfg
  diagnostic/diagnostic.cpp
  frontend/registry.cpp
  frontend/swift.cpp
  printers/dot_printer.cpp 
  main.cpp   
)

target_link_libraries(swift_cfg PRIVATE tree-sitter tree-sitter-swift)
target_include_directories(swift_cfg PRIVATE
  ${CMAKE_SOURCE_DIR}/vendor/tree-sitter/lib/include
  ${SWIFT_GEN_DIR}/src
  ${CMAKE_SOURCE_DIR}/src
)