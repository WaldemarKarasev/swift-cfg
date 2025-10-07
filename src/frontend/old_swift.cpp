#include <frontend/swift.hpp>

#include <tree_sitter/api.h>
#include <iostream>
#include <string>
#include <cstring>

extern "C" const TSLanguage *tree_sitter_swift();

namespace pma::frontends::swift
{

std::unique_ptr<ast::BlockStmt> SwiftTreeSitterAstBuilder::BuildFromRoot(const utils::SourceView& source_code, pma::diagnostic::IDiagnostic& diag)
{
    // parser creation
    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_swift());

    // parsing
    TSTree *tree = ts_parser_parse_string(
        parser,
        nullptr,
        source_code.GetBuffer().data(),
        (uint32_t)source_code.GetBuffer().size()
    );

    // root node
    TSNode root = ts_tree_root_node(tree);

    // tree -> stdout
    char *s_expr = ts_node_string(root);
    std::cout << "Syntax tree:\n" << s_expr << std::endl;
    free(s_expr);

    // free-up memory
    ts_tree_delete(tree);
    ts_parser_delete(parser);

    return std::make_unique<ast::BlockStmt>();
}

} // namespace pma::frontends::swift