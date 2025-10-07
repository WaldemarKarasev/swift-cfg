#include <printers/json_tree_serializer.hpp>

// std
#include <iostream>

// nlohmann json
#include <nlohmann/json.hpp>

namespace pma::printers
{

using json_type = nlohmann::ordered_json;

std::string KindToString(ast::Stmt::Kind kind)
{
    using Kind = ast::Stmt::Kind;
    switch (kind)
    {
    case Kind::Block:       return "BlockStmt";
    case Kind::If:          return "IfStmt";
    case Kind::While:       return "WhileStmt";
    case Kind::For:         return "ForStmt";
    case Kind::DoWhile:     return "DoWhileStmt";
    case Kind::Return:      return "ReturnStmt";
    case Kind::Break:       return "BreakStmt";
    case Kind::Continue:    return "ContinueStmt";
    case Kind::Fallthrough: return "FallthroughStmt";
    case Kind::Expr:        return "ExprStmt";
    case Kind::Switch:      return "SwitchStmt";
    case Kind::SwitchCase:  return "SwitchCaseStmt";
    case Kind::Lable:       return "LableStmt";
    default:
        break;
    }
    return "Unknown";
}

static json_type JStmt(const ast::Stmt& stmt); // fwd


static json_type JBlockStmt(const ast::BlockStmt& block)
{
    json_type array = json_type::array();

    std::cout << "block.stmts_.size()=" << block.stmts_.size() << std::endl;
    for (auto& stmt : block.stmts_)
    {
        array.push_back(stmt ? JStmt(*stmt) : json_type(nullptr));
    }

    return json_type{
        {"kind", KindToString(block.kind_)},
        {"stmts", std::move(array)}
    };
}

static json_type JIfStmt(const ast::IfStmt& if_stmt)
{
    return json_type{
        {"kind" , KindToString(if_stmt.kind_)},
        {"cond", if_stmt.cond},
        {"then", if_stmt.thenB ? JBlockStmt(*if_stmt.thenB) : json_type(nullptr) },
        {"else", if_stmt.elseB ? JBlockStmt(*if_stmt.elseB) : json_type{nullptr} },
    };    
}

static json_type JWhileStmt(const ast::WhileStmt& while_stmt)
{
    return json_type{
        {"kind", KindToString(while_stmt.kind_)},
        {"cond", while_stmt.cond},
        {"body", while_stmt.body ? JBlockStmt(*while_stmt.body) : json_type(nullptr) },
    };    
}

static json_type JForStmt(const ast::ForStmt& for_stmt)
{
    return json_type{
        {"kind", KindToString(for_stmt.kind_)},
        {"item", for_stmt.item},
        {"collection", for_stmt.collection},
        {"body", for_stmt.body ? JBlockStmt(*for_stmt.body) : json_type(nullptr) },
    };
}

static json_type JDoWhileStmt(const ast::DoWhileStmt& do_while_stmt)
{
    return json_type{
        {"kind", KindToString(do_while_stmt.kind_)},
        {"cond", do_while_stmt.cond},
        {"body", do_while_stmt.body ? JBlockStmt(*do_while_stmt.body) : json_type(nullptr)},
    };
}

static json_type JControlStmt(const ast::ControlStmt& ctrl_stmt)
{
    return json_type{
        {"kind", KindToString(ctrl_stmt.kind_)},
        {"has_lable", ctrl_stmt.has_lable},
        {"lable", ctrl_stmt.lable},
    };
}

static json_type JExprStmt(const ast::ExprStmt& expr_stmt)
{
    return json_type{
        {"kind", KindToString(expr_stmt.kind_)},
        {"expr", expr_stmt.expr},
    };
}

static std::string TermToString(ast::SwitchCaseStmt::Terminator term)
{
    using Term = ast::SwitchCaseStmt::Terminator;
    switch (term)
    {
    case Term::None:        return "None";
    case Term::Break:       return "Break";
    case Term::Fallthrough: return "Fallthrough";
    default:
        break;
    }

    return "Unknown";
}

static json_type JSwitchCaseStmt(const ast::SwitchCaseStmt& switch_case_stmt)
{
    return json_type{
        {"kind", KindToString(switch_case_stmt.kind_)},
        {"term", TermToString(switch_case_stmt.terminator)},
        {"default", switch_case_stmt.is_default},
        {"pattern", switch_case_stmt.pattern},
        {"quard", switch_case_stmt.guard ? JExprStmt(*switch_case_stmt.guard) : json_type(nullptr)},
        {"body", switch_case_stmt.body ? JBlockStmt(*switch_case_stmt.body) : json_type(nullptr) },
    };
}

static json_type JSwitchStmt(const ast::SwitchStmt& switch_stmt)
{
    json_type cases = json_type::array();
    for (const auto& case_stmt : switch_stmt.cases)
    {
        cases.push_back(JSwitchCaseStmt(*case_stmt));
    }

    return json_type{
        {"kind", KindToString(switch_stmt.kind_)},
        {"cond", switch_stmt.condition},
        {"cases", std::move(cases)},
    };
}


static json_type JLableStmt(const ast::LableStmt& lable_stmt)
{
    return json_type{
        {"kind", KindToString(lable_stmt.kind_)},
        {"lable", lable_stmt.lable},
    };
}

static json_type JStmt(const ast::Stmt& stmt)
{
    using Kind = ast::Stmt::Kind;
    switch (stmt.kind_)
    {
    case Kind::Block:       return JBlockStmt(static_cast<const ast::BlockStmt&>(stmt));
    case Kind::If:          return JIfStmt(static_cast<const ast::IfStmt&>(stmt));
    case Kind::While:       return JWhileStmt(static_cast<const ast::WhileStmt&>(stmt));
    case Kind::For:         return JForStmt(static_cast<const ast::ForStmt&>(stmt));
    case Kind::DoWhile:     return JDoWhileStmt(static_cast<const ast::DoWhileStmt&>(stmt));
    case Kind::Return:      return JControlStmt(static_cast<const ast::ControlStmt&>(stmt));
    case Kind::Break:       return JControlStmt(static_cast<const ast::ControlStmt&>(stmt));
    case Kind::Continue:    return JControlStmt(static_cast<const ast::ControlStmt&>(stmt));
    case Kind::Fallthrough: return JControlStmt(static_cast<const ast::ControlStmt&>(stmt));
    case Kind::Expr:        return JExprStmt(static_cast<const ast::ExprStmt&>(stmt));
    case Kind::Switch:      return JSwitchStmt(static_cast<const ast::SwitchStmt&>(stmt));
    case Kind::SwitchCase:  return JSwitchCaseStmt(static_cast<const ast::SwitchCaseStmt&>(stmt));
    case Kind::Lable:       return JLableStmt(static_cast<const ast::LableStmt&>(stmt));
    default:
        break;
    }
    return {{"kind", "unknown"}};
}

void JsonTreeSerializer::Print(const ast::Stmt& root)
{
    json_type j = JStmt(root);

    std::cout << j.dump(2) << std::endl;
}

void JsonTreeSerializer::Print(std::ostream& os, const ast::Stmt& root)
{
    os << JStmt(root).dump(2) << std::endl;
}

} // namespace pma::printers
