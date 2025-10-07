// std
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

// tree-sitter api
#include <tree_sitter/api.h>

// pma
#include <frontend/swift.hpp>
#include <printers/json_tree_serializer.hpp>


// forward: tree-sitter-swift
extern "C" const TSLanguage *tree_sitter_swift();

namespace pma::frontends::swift {

namespace {

// field names
const char* statements_name = "statements";
const char* condition_name = "condition";
const char* if_statement_name = "if_statement";
const char* else_name = "else";
const char* while_statement_name = "while_statement";
const char* repeat_while_statement_name = "repeat_while_statement";
const char* for_statement_name = "for_statement";
const char* item_name = "item";
const char* collection_name = "collection";
const char* switch_statement_name = "switch_statement";
const char* switch_entry_name = "switch_entry";
const char* switch_pattern_name = "switch_pattern";
const char* fallthrough_statement_name = "fallthrough_statement"; 
const char* where_keyword_name = "where_keyword";
const char* default_keyword_name = "default_keyword";
const char* statement_label_name = "statement_label";
const char* control_transfer_statement_name = "control_transfer_statement";

inline TSNode field(TSNode n, const char* name) 
{
    return ts_node_child_by_field_name(n, name, std::strlen(name));
}

inline bool is_null(TSNode n) { return ts_node_is_null(n); }
inline bool is_named(TSNode n) { return ts_node_is_named(n); }

inline std::string node_type(TSNode n) 
{
    const char* t = ts_node_type(n);
    return t ? std::string(t) : std::string{};
}

inline utils::SourceRange rng(TSNode n) 
{
    return utils::SourceRange{
        .start_byte = ts_node_start_byte(n),
        .end_byte   = ts_node_end_byte(n)
    };
}
inline std::string_view text_of(const utils::SourceView& sv, TSNode n) 
{
    return sv.slice(rng(n));
}

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

inline TSNode as_code_block_body(TSNode block) { return block; }

// ----- builders (For diag) -----

std::unique_ptr<ast::Stmt> build_stmt(TSNode n, const utils::SourceView& sv); // fwd
std::unique_ptr<ast::BlockStmt> build_block(TSNode n, const utils::SourceView& sv); // fwd

std::unique_ptr<ast::IfStmt> build_if(TSNode n, const utils::SourceView& sv) 
{
    auto out = std::make_unique<ast::IfStmt>();
    TSNode cond = field(n, condition_name);
    out->cond  = std::string(text_of(sv, cond)); // conditional statement string
    out->condR = rng(cond); // conditional source range
    
    // std::cout << "======================" << std::endl;
    bool else_exists = false;
    for (const auto child : named_children(n))
    {
        std::cout << "node_type: " << node_type(child) << std::endl;//<< "; text: " << text_of(sv, child) << std::endl;

        for (const auto child_next : named_children(child))
        {
            std::cout << "   child_node: " << node_type(child_next) << std::endl;
        }

        const auto t = node_type(child);
        if (t == else_name)
        {
            else_exists = true;
            // probably will be an 
        }
        if (t == statements_name)
        {
            if (else_exists)
            {
                std::cout << "Building simple else stmt block" << std::endl;
                out->elseB = build_block(child, sv);
            }
            else
            {
                std::cout << "Building if stmt block" << std::endl;
                out->thenB = build_block(child, sv);
            }
        }
        if (t == if_statement_name)
        {
            if (else_exists)
            {
                auto block = std::make_unique<ast::BlockStmt>();
                block->stmts_.push_back(build_if(child, sv));
                out->elseB = std::move(block);
            }
            else
            {
                std::cout << "Unable to build nested if statement block without else flag raised" << std::endl;
            }
        }

    }
    // std::cout << "======================" << std::endl;


    return out;
}

std::unique_ptr<ast::WhileStmt> build_while(TSNode n, const utils::SourceView& sv) 
{
    auto out = std::make_unique<ast::WhileStmt>();
    TSNode cond = field(n, condition_name);
    out->cond  = std::string(text_of(sv, cond));
    out->condR = rng(cond);

    TSNode body = field(n, statements_name);
    if (!is_null(body) )
    {
        out->body = build_block(body, sv);
    }
    return out;
}

std::unique_ptr<ast::DoWhileStmt> build_repeat_while(TSNode n, const utils::SourceView& sv) 
{
    auto out = std::make_unique<ast::DoWhileStmt>();
    
    // conditions at the end
    TSNode cond = field(n, condition_name);
    out->cond  = std::string(text_of(sv, cond));
    out->condR = rng(cond);

    // statements in repeat block
    TSNode body = field(n, statements_name);
    if (!is_null(body))
    {
        out->body = build_block(body, sv);
    }
    return out;
}

std::unique_ptr<ast::ForStmt> build_for_in(TSNode n, const utils::SourceView& sv) 
{
    auto out = std::make_unique<ast::ForStmt>();
    TSNode item  = field(n, item_name);
    TSNode collection = field(n, collection_name); 
    
    out->item           = std::string(text_of(sv, item));
    out->itemR          = rng(item);
    out->collection     = std::string(text_of(sv, collection));
    out->collectionR    = rng(collection);

    TSNode body = field(n, statements_name);
    if (!is_null(body))
    {
        out->body = build_block(body, sv);
    }
    return out;
}

std::unique_ptr<ast::SwitchStmt> build_switch(TSNode n, const utils::SourceView& sv) 
{
    auto out = std::make_unique<ast::SwitchStmt>();


    TSNode switch_expr = ts_node_child_by_field_name(n, "expr", strlen("expr"));
    if (!is_null(switch_expr))
    {
        out->condition = text_of(sv, switch_expr);
        out->condition_range = rng(switch_expr);
    }

    for (auto c : named_children(n)) 
    {
        const auto t = node_type(c);
        if (t == switch_entry_name)
        {
            auto sc = std::make_unique<ast::SwitchCaseStmt>();
            sc->terminator = ast::SwitchCaseStmt::Terminator::Break; // swift default behaviour
            
            // child named children
            for (auto entry_children : named_children(c))
            {
                const auto t = node_type(entry_children);

                if (t == switch_pattern_name)
                {
                    // switch_pattern
                    sc->pattern = text_of(sv, entry_children);
                }

                if (t == where_keyword_name)
                {
                    // where stmt
                    auto guard = std::make_unique<ast::ExprStmt>();
                    guard->expr  = std::string(text_of(sv, entry_children));
                    guard->exprR = rng(entry_children);
                }

                if (t == statements_name)
                {
                    // statements
                    sc->body = build_block(entry_children, sv);

                }

                if (t == default_keyword_name)
                {
                    sc->is_default = true;
                }
            }

            // unnamed children. Trying to find fallthrough statement
            for (uint32_t i = 0; i < ts_node_child_count(c); ++i)
            {
                TSNode child = ts_node_child(c, i);
                if (node_type(child) == "fallthrough")
                {
                    auto fallthrough_stmt = std::make_unique<ast::FallthroughStmt>();
                    sc->body->stmts_.push_back(std::move(fallthrough_stmt));
                    sc->terminator = ast::SwitchCaseStmt::Terminator::Fallthrough;
                }
            }

            out->cases.push_back(std::move(sc));
        }
    }
    return out;
}


std::unique_ptr<ast::Stmt> build_control_statement(TSNode n, const utils::SourceView& sv) 
{
    std::unique_ptr<ast::ControlStmt> control_stmt = nullptr;
    std::string lable{};
    utils::SourceRange lableR{};
    for (uint32_t i = 0; i < ts_node_child_count(n); ++i)
    {
        TSNode child = ts_node_child(n, i);

        const char* field_name = ts_node_field_name_for_child(n, i);
        if (field_name == nullptr)
        { 
            const auto unnamed_type = node_type(child);
            if (unnamed_type == "return")   control_stmt = std::make_unique<ast::ReturnStmt>();
            if (unnamed_type == "continue") control_stmt = std::make_unique<ast::ContinueStmt>();
            if (unnamed_type == "break")    control_stmt = std::make_unique<ast::BreakStmt>();
        }
        else
        {
            lable = node_type(child);
            lableR = rng(child);
        }
    }
    
    if (control_stmt != nullptr && !lable.empty())
    {
        control_stmt->lable = std::move(lable);
        control_stmt->has_lable = true;
        control_stmt->lableR = lableR;
    }

    return control_stmt;
}


std::unique_ptr<ast::BlockStmt> build_block(TSNode n, const utils::SourceView& sv) 
{
    auto blk = std::make_unique<ast::BlockStmt>();
    TSNode body = as_code_block_body(n);
    for (auto s : named_children(body)) 
    {
        blk->stmts_.push_back(build_stmt(s, sv));
    }
    return blk;
}

std::unique_ptr<ast::LableStmt> build_lable(TSNode n, const utils::SourceView& sv) 
{
    auto lable = std::make_unique<ast::LableStmt>();
    
    lable->lable = std::string(text_of(sv, n));
    lable->lableR = rng(n);

    return lable;
}

std::unique_ptr<ast::Stmt> build_stmt(TSNode n, const utils::SourceView& sv) {
    const auto t = node_type(n);

    if (t == statements_name)                   return build_block(n, sv);
    if (t == if_statement_name)                 return build_if(n, sv);
    if (t == while_statement_name)              return build_while(n, sv);
    if (t == repeat_while_statement_name)       return build_repeat_while(n, sv);
    if (t == for_statement_name)                return build_for_in(n, sv);
    if (t == switch_statement_name)             return build_switch(n, sv);

    if (t == control_transfer_statement_name)   return build_control_statement(n, sv);
    if (t == statement_label_name)              return build_lable(n, sv);

    // As expresion by default
    auto e = std::make_unique<ast::ExprStmt>();
    e->expr  = std::string(text_of(sv, n));
    e->exprR = rng(n);
    return e;
}

} // namespace

// -------- entrypoint --------

std::unique_ptr<ast::BlockStmt>
SwiftTreeSitterAstBuilder::BuildFromRoot(const utils::SourceView& source_code,
                                         pma::diagnostic::IDiagnostic& diag)
{
    // 1) parser
    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_swift());

    // 2) parse
    const std::string& buffer = source_code.GetBuffer();
    TSTree* tree = ts_parser_parse_string(
        parser,
        nullptr,
        buffer.c_str(),
        static_cast<uint32_t>(buffer.size())
    );

    // 3) root 
    TSNode root = ts_tree_root_node(tree);
    if (ts_node_has_error(root)) {
    
        pma::diagnostic::IDiagnostic::SourceRangeType rng;
        rng = pma::utils::SourceRange{ts_node_start_byte(root), ts_node_end_byte(root)};
        diag.error("Parse error in Swift source", rng);
    
        auto empty = std::make_unique<ast::BlockStmt>();
        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return empty;
    }

    // 4) Top level BlockStmt
    auto program = std::make_unique<ast::BlockStmt>();
    for (auto child : named_children(root)) {
        program->stmts_.push_back(build_stmt(child, source_code));
    }  
    
    // 5) cleanup
    ts_tree_delete(tree);
    ts_parser_delete(parser);

    return program;
}

} // namespace pma::frontends::swift
