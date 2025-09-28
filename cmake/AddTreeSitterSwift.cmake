cmake_minimum_required(VERSION 3.16)

# tree-sitter-swift setup
set(SWIFT_GRAMMAR_SRC_DIR "${CMAKE_SOURCE_DIR}/vendor/tree-sitter-swift")

# Finding CLI tree-sitter 
find_program(TREE_SITTER_CLI
  NAMES tree-sitter tree-sitter.exe tree-sitter.cmd
  DOC "tree-sitter CLI to generate parser.c from grammar.js"
)

if(NOT TREE_SITTER_CLI)
  message(FATAL_ERROR "tree-sitter CLI not found. Install with 'npm install' from vendor/tree-sitter-swift directory")
endif()

message(AAAA ${CMAKE_BINARY_DIR})

# generation directory for the tree-sitter-swift
set(SWIFT_GEN_DIR "${CMAKE_BINARY_DIR}/tree-sitter-swift")

# tree-sitter-swift output files setup 
set(SWIFT_PARSER_C   "${SWIFT_GEN_DIR}/src/parser.c")
set(SWIFT_SCANNER_C  "${SWIFT_GEN_DIR}/src/scanner.c")   

# Generating command:
#    - Copying all files to build/gen
#    - Run tree-sitter generate 
add_custom_command(
  OUTPUT "${SWIFT_PARSER_C}" "${SWIFT_SCANNER_C}"
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${SWIFT_GEN_DIR}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${SWIFT_GEN_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${SWIFT_GRAMMAR_SRC_DIR}" "${SWIFT_GEN_DIR}"
  COMMAND ${CMAKE_COMMAND} -E chdir "${SWIFT_GEN_DIR}" ${TREE_SITTER_CLI} generate
  DEPENDS "${SWIFT_GRAMMAR_SRC_DIR}/grammar.js"
  BYPRODUCTS "${SWIFT_PARSER_C}" "${SWIFT_SCANNER_C}"
  COMMENT "Generating Swift parser.c with tree-sitter CLI"
)


# Generate swift parser
add_custom_target(gen-swift-parser ALL
  DEPENDS "${SWIFT_PARSER_C}" "${SWIFT_SCANNER_C}"
)

# Lib for swift parser
add_library(tree-sitter-swift STATIC
  "${SWIFT_PARSER_C}"
  "${SWIFT_SCANNER_C}"
  "${SWIFT_SCANNER_CC}"
)

add_dependencies(tree-sitter-swift gen-swift-parser)