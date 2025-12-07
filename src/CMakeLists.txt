cmake_minimum_required(VERSION 3.16)
project(swift-cfg LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)


add_executable(swift_cfg
  cfg/cfg_builder.cpp
  diagnostic/diagnostic.cpp
  frontend/registry.cpp
  frontend/swift.cpp
  printers/dot_printer.cpp 
  printers/json_tree_serializer.cpp 
  app.cpp
  main.cpp   
)

target_link_libraries(swift_cfg PRIVATE tree-sitter tree-sitter-swift nlohmann_json::nlohmann_json)
target_include_directories(swift_cfg PRIVATE
  ${CMAKE_SOURCE_DIR}/vendor/tree-sitter/lib/include
  ${SWIFT_GEN_DIR}/src
  ${CMAKE_SOURCE_DIR}/src
)

add_executable(playground
  playground.cpp   
)
target_link_libraries(playground PRIVATE tree-sitter tree-sitter-swift)


add_executable(code-metrics
  diagnostic/diagnostic.cpp
  frontend/registry.cpp
  frontend/swift.cpp
  printers/json_tree_serializer.cpp
  code_metric.cpp
  metric_app.cpp
)
target_link_libraries(code-metrics PRIVATE tree-sitter tree-sitter-swift nlohmann_json::nlohmann_json)
target_include_directories(code-metrics PRIVATE
  ${CMAKE_SOURCE_DIR}/vendor/tree-sitter/lib/include
  ${SWIFT_GEN_DIR}/src
  ${CMAKE_SOURCE_DIR}/src
)