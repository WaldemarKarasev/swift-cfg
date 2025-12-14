// std
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <regex>

// tree-sitter api
#include <tree_sitter/api.h>

// pma
#include <frontend/swift.hpp>
#include <printers/json_tree_serializer.hpp>


// forward: tree-sitter-swift
extern "C" const TSLanguage *tree_sitter_swift();

namespace pma::frontends::swift {

namespace {

std::string expand_ranges(const std::string& input) 
{
    std::regex range_regex(R"((\-?\d+)\.\.\.(\-?\d+))");
    std::string result;
    std::sregex_iterator it(input.begin(), input.end(), range_regex);
    std::sregex_iterator end;

    size_t last_pos = 0;
    for (; it != end; ++it) 
    {
        const auto& match = *it;
        int start = std::stoi(match[1]);
        int endv  = std::stoi(match[2]);

        result.append(input.substr(last_pos, match.position() - last_pos));

        std::string expanded;
        if (start <= endv) 
        {
            for (int i = start; i <= endv; ++i) 
            {
                if (!expanded.empty()) expanded += ",";
                expanded += std::to_string(i);
            }
        } 
        else
        {
            for (int i = start; i >= endv; --i) 
            {
                if (!expanded.empty()) expanded += ",";
                expanded += std::to_string(i);
            }
        }

        result.append(expanded);
        last_pos = match.position() + match.length();
    }

    result.append(input.substr(last_pos));
    return result;
}

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
const char* simple_identifier_name = "simple_identifier";
const char* function_declaration_name = "function_declaration";
const char* parameter_name = "parameter";

// class labels
const char* class_decl = "class_declaration";
const char* inheritance_specifier = "inheritance_specifier";
const char* init_decl = "init_declaration";
const char* deinint_decl = "deinit_declaration";
const char* property_decl = "property_declaration";
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

inline std::vector<TSNode> children(TSNode n)
{
    std::vector<TSNode> children;
    uint32_t count = ts_node_child_count(n);
    for (uint32_t i = 0; i < count; ++i) 
    {
        children.push_back(ts_node_child(n, i));
    }
    return children;
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
        // std::cout << "node_type: " << node_type(child) << std::endl;//<< "; text: " << text_of(sv, child) << std::endl;

        for (const auto child_next : named_children(child))
        {
            // std::cout << "   child_node: " << node_type(child_next) << std::endl;
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
                // std::cout << "Building simple else stmt block" << std::endl;
                out->elseB = build_block(child, sv);
            }
            else
            {
                // std::cout << "Building if stmt block" << std::endl;
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
                // std::cout << "Unable to build nested if statement block without else flag raised" << std::endl;
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


    // statements in repeat block
    for (auto child : named_children(n))
    {
        if (node_type(child) == statements_name)
        {
            out->body = build_block(child, sv);
        }
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
    for (auto child : named_children(n))
    {
        if (node_type(child) == statements_name)
        {
            out->body = build_block(child, sv);
        }
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

    std::string collection_seq= {};// = expand_ranges(std::string(text_of(sv, collection)));

    if (!collection_seq.empty())
    {
        out->collection = collection_seq;
    }
    else
    {
        out->collection     = std::string(text_of(sv, collection));
    }
    out->collectionR    = rng(collection);

    for (auto child : named_children(n))
    {
        if (node_type(child) == statements_name)
        {
            out->body = build_block(child, sv);
        }
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
                    if (sc->pattern.empty())
                    {
                        std::string extended = {};//expand_ranges(std::string(text_of(sv, entry_children)));
                        if (!extended.empty())
                        {
                            sc->pattern = extended;
                        }
                        else
                        {
                            sc->pattern = text_of(sv, entry_children);
                        }
                    } 
                    else
                    {
                        sc->pattern += ", ";
                        sc->pattern += text_of(sv, entry_children);
                    } 
                }

                if (t == where_keyword_name)
                {
                    // std::cout << "where caught. text: " << std::endl;
                    // where stmt
                    auto guard = std::make_unique<ast::ExprStmt>();
                    sc->guard = std::move(guard);
                }

                if (sc->guard != nullptr && t != where_keyword_name)
                {
                    // std::cout << "where expression detected. type: " << node_type(entry_children) << std::endl;
                    if (std::string(t).find("expression") != std::string::npos)
                    {
                        sc->guard->text = std::string(text_of(sv, entry_children));
                        sc->guard->expr = std::make_unique<ast::UnknownExpr>();
                        sc->guard->expr->range = rng(entry_children);
                        // std::cout << "where expresiion: " << sc->guard->text << std::endl;
                    }
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
        
        if (TSNode node = field(n, "result"); !is_null(node))
        {
            lable = text_of(sv, node);
            lableR = rng(node);
            // std::cout << "ast lable=" << lable << std::endl;

        }
        // if (node_type(child) == simple_identifier_name)
        // {        
        //     // std::cout << "node_type:" << node_type(child) << "; text_of:" << text_of(sv, child) << std::endl;
        //     // std::cout << "node_type:" << node_type(child) << std::endl;
        //     lable = text_of(sv, child);
        //     lableR = rng(child);
        // }
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

static std::string trim(std::string s)
{
    auto ws = [](unsigned char c){ return std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    [&](unsigned char c){ return !ws(c); }));
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [&](unsigned char c){ return !ws(c); }).base(), s.end());

    while (!s.empty() && s.back() == ':')
        s.pop_back();

    return s;
}


std::unique_ptr<ast::LableStmt> build_lable(TSNode n, const utils::SourceView& sv) 
{
    auto lable = std::make_unique<ast::LableStmt>();
    
    lable->lable = trim(std::string(text_of(sv, n)));
    lable->lableR = rng(n);
    // std::cout << "ast lable.lable=" << lable->lable << std::endl;

    return lable;
}

std::unique_ptr<ast::FunctionDeclStmt> build_func_decl(TSNode n, const utils::SourceView& sv) 
{
    auto func = std::make_unique<ast::FunctionDeclStmt>();
    // if (node_type(n) == init_decl || node_type(n) == deinint_decl)
    // {
    //     func->signature.name = 
    // }
    std::cout << "func::node_type: " << node_type(n) << std::endl;
    std::string sig_name;
    utils::SourceRange sig_rng;
    std::string modifiers;
    for (uint32_t i = 0; i < ts_node_child_count(n); ++i)
    {
        TSNode child = ts_node_child(n, i);
        // std::cout << "~~~func_child::node_type: " << node_type(child) << std::endl;
        const auto t = node_type(child);
        if (t == simple_identifier_name)
        {
            sig_name = text_of(sv, child);
            sig_rng = rng(child);
        }

        if (t == "init")
        {
            sig_name = text_of(sv, child);
            sig_rng = rng(child);
            func->is_initializer = true;
        }

        if (t == "deinit")
        {
            sig_name = text_of(sv, child);
            sig_rng = rng(child);
            func->is_deinitializer = true;
        }

        if (t == "modifiers")
        {
            for (uint32_t j = 0; j < ts_node_child_count(child); ++j)
            {        
                // TODO: correct ranges addition
                // std::cout << "modifier: " << node_type(ts_node_child(child, j)) << std::endl;
                std::string text = std::string(text_of(sv, ts_node_child(child, j)));
                if (text == "override") func->is_override = true;
                modifiers.append(text + " ");
            }
            // std::cout << "collected modifiers: " << modifiers << std::endl;

        }

        if (t == parameter_name)
        {
            ast::Param param{}; 
            
            for (auto param_child : named_children(child))
            {
                if (node_type(param_child) == simple_identifier_name)
                {
                    std::vector<const char*> field_names = {"external_name", "name"};
                    
                    for (const char* name : field_names)
                    {
                        auto node = field(child, name);
                        if (!is_null(node))
                        {
                            if (name == "external_name")
                            {
                                param.external_name = text_of(sv, node);
                                param.external_nameR = rng(node);
                            }
                            if (name == "name")
                            {
                                param.local_name = text_of(sv, node);
                                param.local_nameR = rng(node);
                            }
                        }
                    }
                }
                if (node_type(param_child) == "user_type")
                {
                    param.type_name = text_of(sv, param_child);
                    param.type_nameR = rng(param_child);
                }
                if (node_type(param_child) == "optional_type")
                {
                    param.type_name = text_of(sv, param_child);
                    param.type_nameR = rng(param_child);
                }
            }
            func->signature.params.push_back(std::move(param));
        }

        if (t == "user_type")
        {
            func->signature.return_type = text_of(sv, child);
            func->signature.return_typeR = rng(child);
        }

        if (t == "function_body")
        {
            for (auto body : named_children(child))
            {
                if (node_type(body) == statements_name)
                {
                    func->body = build_block(body, sv);
                }
            }
        }
    }

    func->signature.name = sig_name;
    func->signature.name = modifiers + func->signature.name;
    func->signature.nameR = sig_rng;
    return func;
}

std::unique_ptr<ast::ExprStmt> build_expr_stmt(TSNode n, const utils::SourceView& sv);

std::unique_ptr<ast::VarDeclStmt> build_vardecl(TSNode n, const utils::SourceView& sv)
{
    std::cout << "build_vardecl: " << node_type(n) << std::endl;
    auto var_decl = std::make_unique<ast::VarDeclStmt>();

    for (uint32_t i = 0; i < ts_node_child_count(n); ++i)
    {
        TSNode child = ts_node_child(n, i);
        // std::cout << "node_type: " << node_type(child) << " \"" << text_of(sv, child) << "\"" << std::endl;

        if (node_type(child) == "pattern")
        {
            var_decl->name = text_of(sv, child);
            var_decl->nameR = rng(child);
        }
        if (node_type(child) == "type_annotation")
        {
            TSNode user_type = field(child, "name");
            if (!ts_node_is_null(user_type))
            {
                var_decl->type_name = text_of(sv, user_type);
                var_decl->type_nameR = rng(user_type);
            }
            else
            {
                var_decl->type_name = text_of(sv, child);
                var_decl->type_nameR = rng(child);
            }
        }
    }

    if (auto name = field(n, "value"); !ts_node_is_null(name))
    {
        var_decl->initializer = build_expr_stmt(name, sv);
        // auto init = std::make_unique<ast::ExprStmt>();
        // init->text = text_of(sv, name);
        // init->range = rng(name);
        // var_decl->initializer = std::move(init);
    }

    return var_decl;
}

std::unique_ptr<ast::ClassDeclStmt> build_class_decl(TSNode n, const utils::SourceView& sv)
{
    std::cout << "class decl: nodetext: " << node_type(n) << std::endl; 

    auto class_decl = std::make_unique<ast::ClassDeclStmt>();

    if (auto name = field(n, "name"); !ts_node_is_null(name)) 
    {
        class_decl->name = text_of(sv, name);
        class_decl->range = rng(name);
        // std::cout << "name: " << text_of(sv, name) << ", type: " << node_type(name) << std::endl;
    }

    uint32_t count = ts_node_child_count(n);
    for (uint32_t i = 0; i < count; ++i) 
    {
        TSNode child = ts_node_child(n, i);
        if (node_type(child) == inheritance_specifier) 
        {
            class_decl->base_class_name = text_of(sv, child);
            class_decl->base_class_nameR = rng(child);
        }
    }

    if (auto body = field(n, "body"); !ts_node_is_null(body)) 
    {
        // class_decl->name = text_of(sv, body);
        // class_decl->range = rng(name);
        std::cout << "body: " << node_type(body) << ", type: " << node_type(body) << std::endl;

        for (auto child : named_children(body))
        {
            std::cout << "type: " << node_type(child) << std::endl;
            if (node_type(child) == init_decl ||
                node_type(child) == deinint_decl ||
                node_type(child) == function_declaration_name)
            {
                class_decl->members.push_back(build_func_decl(child, sv));
            }

            if (node_type(child) == property_decl)
            {
                auto var_decl = build_vardecl(child, sv);
                var_decl->is_property = true;
                class_decl->members.push_back(std::move(var_decl));

            }
        }
    }
    return class_decl;
}

std::unique_ptr<ast::ExprBase> build_expr(TSNode n, const utils::SourceView& sv);
std::unique_ptr<ast::ExprBase> build_call_expr(TSNode n, const utils::SourceView& sv);
std::unique_ptr<ast::ExprBase> build_member_access_expr(TSNode n, const utils::SourceView& sv);

std::unique_ptr<ast::ExprBase> build_identifier_expr(TSNode n, const utils::SourceView& sv)
{
    auto ident = std::make_unique<ast::IdentifierExpr>();
    ident->name = text_of(sv, n);
    ident->nameR = rng(n);
    return ident;
}

std::unique_ptr<ast::ExprBase> build_member_access_expr(TSNode n, const utils::SourceView& sv)
{
    auto member_expr = std::make_unique<ast::MemberAccessExpr>();

    if (TSNode target = field(n, "target"); !ts_node_is_null(target))
    {
        if (node_type(target) == "self_expression")
        {
            member_expr->base = build_identifier_expr(target, sv);

        }
        if (node_type(target) == "super_expression")
        {
            member_expr->base = build_identifier_expr(target, sv);

        }
        if (member_expr == nullptr)
        {
            for (auto& child : children(target))
            {
                if (node_type(child) == "self_expression")
                {
                    member_expr->base = build_identifier_expr(child, sv);
                    break;
                }
            }
        }
    }

    if (TSNode suffix = field(n, "suffix"); !ts_node_is_null(suffix))
    {
        if (node_type(suffix) == "navigation_suffix")
        {
            for (auto& child : children(suffix))
            {
                if (node_type(child) == simple_identifier_name)
                {
                    member_expr->member_name = text_of(sv, child);
                    member_expr->member_nameR = rng(child);
                    break;
                }
            }
        }
        else
        {
            member_expr->member_name = text_of(sv, suffix);
            member_expr->member_nameR = rng(suffix);
        }
    }

    return member_expr;
}

std::vector<std::unique_ptr<ast::ExprBase>> build_call_suffix_args(TSNode call_suffix, const utils::SourceView& sv)
{
    std::vector<std::unique_ptr<ast::ExprBase>> args;

    TSNode arguments;
    for (auto& child : children(call_suffix))
    {
        if (node_type(child) == "value_arguments")
        {
            arguments = child;
        }
    }

    if (not ts_node_is_null(arguments))
    {
        for (auto& arg : children(arguments))
        {
            if (TSNode value = field(arg, "value"); !ts_node_is_null(value))
            {
                args.push_back(build_expr(value, sv));
            }

        }
    }

    return args;
}


std::unique_ptr<ast::ExprBase> build_call_expr(TSNode n, const utils::SourceView& sv)
{
    auto call_expr = std::make_unique<ast::CallExpr>();

    for (auto& child : children(n))
    {
        if (node_type(child) == "navigation_expression")
        {
            call_expr->callee = build_member_access_expr(child, sv);
        }
        if (node_type(child) == simple_identifier_name)
        {
            call_expr->callee = build_identifier_expr(child, sv);
        }
        if (node_type(child) == "call_suffix")
        {
            call_expr->args = build_call_suffix_args(child, sv);
        }
    }

    return call_expr;
}


std::unique_ptr<ast::ExprBase> build_expr(TSNode n, const utils::SourceView& sv)
{
    std::unique_ptr<ast::ExprBase> expr;

    if (node_type(n) == "assignment")
    {
        if (TSNode target = field(n, "target"); !ts_node_is_null(target))
        {
            for (auto& child : children(target))
            {
                if (node_type(child) == "navigation_expression")
                {
                    expr = build_member_access_expr(child, sv);
                }
                if (node_type(child) == "simple_identifier")
                {
                    expr = build_identifier_expr(child, sv);
                }
            }
        }
    }
    else if (node_type(n) == "call_expression")
    {
        expr = build_call_expr(n, sv);
    }
    else if (node_type(n) == "simple_identifier")
    {
        expr = build_identifier_expr(n, sv);
    } 
    else if (node_type(n) == "navigation_expression")
    {
        expr = build_member_access_expr(n, sv);
    }
    else if (node_type(n) == property_decl)
    {
        // std::cout << "here" << std::endl;
        auto var_decl = build_vardecl(n, sv);
        if (var_decl != nullptr && 
            var_decl->initializer != nullptr && 
            var_decl->initializer->expr != nullptr)
        {
            expr = std::move(var_decl->initializer->expr);
            // expr->text = var_decl->initializer->expr->text;
        }
        else
        {
            // default
            expr = std::make_unique<ast::ExprBase>(ast::ExprBase::Unknown);
            expr->range = rng(n);
            expr->text = text_of(sv, n);
        }
    }
    else
    {
        // default
        expr = std::make_unique<ast::ExprBase>(ast::ExprBase::Unknown);
        expr->range = rng(n);
        expr->text = text_of(sv, n);
    }

    return expr;
}

std::unique_ptr<ast::ExprStmt> build_expr_stmt(TSNode n, const utils::SourceView& sv)
{
    auto e = std::make_unique<ast::ExprStmt>();

    e->expr = build_expr(n, sv);

    e->text = std::string(text_of(sv, n));
    return e;
}

std::unique_ptr<ast::Stmt> build_stmt(TSNode n, const utils::SourceView& sv) 
{
    const auto t = node_type(n);

    std::cout << "build_stmt: " << node_type(n) << std::endl;

    if (t == statements_name)                   return build_block(n, sv);
    if (t == if_statement_name)                 return build_if(n, sv);
    if (t == while_statement_name)              return build_while(n, sv);
    if (t == repeat_while_statement_name)       return build_repeat_while(n, sv);
    if (t == for_statement_name)                return build_for_in(n, sv);
    if (t == switch_statement_name)             return build_switch(n, sv);

    if (t == control_transfer_statement_name)   return build_control_statement(n, sv);
    if (t == statement_label_name)              return build_lable(n, sv);
    if (t == function_declaration_name)         return build_func_decl(n, sv);
    if (t == class_decl)                        return build_class_decl(n, sv);


    // As expresion by default
    return build_expr_stmt(n, sv);
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
