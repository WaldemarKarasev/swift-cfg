#include <tree_sitter/api.h>
#include <iostream>
#include <string>
#include <cstring>

extern "C" const TSLanguage *tree_sitter_swift();

int main() {
    // Swift code
    const char *source_code = R"swift(
        func test(x: Int) -> Int {
            if x > 0 {
                return x
            } else {
                return -x
            }
        }
    )swift";

    // parser creation
    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_swift());

    // parsing
    TSTree *tree = ts_parser_parse_string(
        parser,
        nullptr,
        source_code,
        (uint32_t)strlen(source_code)
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

    return 0;
}

