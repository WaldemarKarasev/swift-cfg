#include <tree_sitter/api.h>
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <fstream>

// forward: tree-sitter-swift
extern "C" const TSLanguage *tree_sitter_swift();

inline TSNode field(TSNode n, const char* name) 
{
    return ts_node_child_by_field_name(n, name, std::strlen(name));
}


inline std::string node_type(TSNode n) 
{
    const char* t = ts_node_type(n);
    return t ? std::string(t) : std::string{};
}

inline bool is_named(TSNode n) { return ts_node_is_named(n); }
inline bool is_null(TSNode n) { return ts_node_is_null(n); }


inline std::vector<TSNode> named_children(TSNode n) 
{
    std::vector<TSNode> out;
    const uint32_t cnt = ts_node_child_count(n);
    out.reserve(cnt);
    for (uint32_t i = 0; i < cnt; ++i) 
    {
        TSNode c = ts_node_child(n, i);
        if (is_named(c)) out.push_back(c);
    }
    return out;
}

#include <algorithm>

inline std::string_view text_of(const std::string& sv, TSNode n) 
{
    uint32_t start = ts_node_start_byte(n);
    uint32_t end = ts_node_end_byte(n);

    const auto number = static_cast<uint32_t>(sv.size());
    const uint32_t s = std::min(start, number);
    const uint32_t e = std::min(end, number);
    if (e <= s) return {};
    return std::string_view(sv.data() + s, e - s);
}

void traverse_tree(const std::string& source, TSNode root, std::string indent)
{
    std::cout << indent << "node_type:" << node_type(root) << "; text_of:" << text_of(source, root) << std::endl;
    for (auto c : named_children(root))
    {
        traverse_tree(source, c, indent + " ");           

    }
}

void traverse_tree_unnamed(const std::string& source, TSNode root, std::string indent)
{
    // std::cout << indent << "|node_type:" << node_type(root) << "; text_of:" << text_of(source, root) << std::endl;
    for (uint32_t i = 0; i < ts_node_child_count(root); ++i)
    {
        TSNode child = ts_node_child(root, i);

        const char* field_name = ts_node_field_name_for_child(root, i);
        if (field_name == nullptr)
        { 
            std::cout << indent << "|Unnamed child:" << ts_node_type(child) << "; text_of:" << text_of(source, child) << std::endl;
        }
        else
        {
            std::cout << indent << "|Named field:" << field_name << "(" << ts_node_type(child) << ") ; text_of:" << text_of(source, child) << std::endl;
        }
        traverse_tree_unnamed(source, child, indent + " ");
    }
}

int main(int argc, char** argv) 
{

    std::string filename;
    if (argc > 1)
    {   
        filename = argv[1];
    }
    else
    {
        filename = "code_examples/for_break_continue.swift";
    }

    std::string source;
    std::ifstream file(filename);
    if (file.is_open())
    {
        std::string str;
        while (std::getline(file, str))
        {
            source += str + "\n";
        }
    }

    std::cout << "CODE FILENAME: " << filename << std::endl;
    if (source.empty())
    {
        std::cout << "failed to read code from file:" << filename << std::endl;
        return -1;
    }
    std::cout << "[CODE START]:\n" << source << "\n[CODE END]" << std::endl;

    // 1) parser
    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_swift());

    // 2) parse
    const std::string& buffer = source;
    TSTree* tree = ts_parser_parse_string(
        parser,
        nullptr,
        buffer.c_str(),
        static_cast<uint32_t>(buffer.size())
    );

    TSNode root = ts_tree_root_node(tree);
    if (ts_node_has_error(root)) {
    
        std::cout << "PARSING ERROR" << std::endl;
        return -1;
    }

    // traverse_tree(source, root, " ");
    traverse_tree_unnamed(source, root, " ");

    ts_node_string(root);

    // 5) cleanup
    ts_tree_delete(tree);
    ts_parser_delete(parser);


}

