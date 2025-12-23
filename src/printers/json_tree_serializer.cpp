#include <printers/json_tree_serializer.hpp>

// std
#include <iostream>

// nlohmann json
#include <nlohmann/json.hpp>

namespace pma::printers
{

using json_type = nlohmann::ordered_json;

// CFG json serialization

template <typename T>
static json_type vec_to_jarr(const std::vector<T>& vec)
{
    json_type arr = json_type::array();

    for (const auto& elem : vec)
    {
        arr.push_back(elem);
    }

    return arr;
}

static json_type JEdges(const std::vector<cfg::Edge>& edges)
{
    json_type arr = json_type::array();
    for (const auto& edge : edges)
    {
        json_type j_edge = json_type{
            {"to", edge.to},
            {"lable", edge.label},
        };

        arr.push_back(std::move(j_edge));
    }

    return arr;
}

static json_type JCfgBlocks(const std::vector<cfg::BasicBlock>& blocks)
{
    json_type arr = json_type::array();
    for (const auto& block : blocks)
    {
        json_type jblock = json_type {
            {"id", block.id},
            {"closed", block.closed},
            {"instr", vec_to_jarr<std::string>(block.instrs)},
            {"outs", JEdges(block.outs)},            
        };

        arr.push_back(std::move(jblock));
    }

    return arr;
}

void JsonTreeSerializer::Print(std::ostream& os, const cfg::Graph& cfg)
{
    json_type cfg_json = json_type{
        {"entry", cfg.entry},
        {"exit", cfg.exit},
        {"blocks", JCfgBlocks(cfg.blocks)}
    };

    os << cfg_json.dump(2);
    return;
}


// AST json serialization

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
    case Kind::Function:    return "Function";
    case Kind::Class:       return "Class";
    case Kind::VarDecl:     return "VarDecl";
    case Kind::Identifier:  return "Identifier";
    default:
        break;
    }
    return "Unknown";
}

std::string EKindToString(ast::ExprBase::Kind kind)
{
    using Kind = ast::ExprBase::Kind;
    switch (kind)
    {
        case Kind::Identifier:      return "Identifier";
        case Kind::MemberAccess:    return "MemberAccess";
        case Kind::Call:            return "Call";

        default:
        case Kind::Unknown:;
    }
    return "Unknown";
}

static json_type JStmt(const ast::Stmt& stmt); // fwd


static json_type JBlockStmt(const ast::BlockStmt& block)
{
    json_type array = json_type::array();

    // std::cout << "block.stmts_.size()=" << block.stmts_.size() << std::endl;
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

// ---------- Expressions ----------
static json_type JExprIdentifier(const ast::IdentifierExpr& ident_expr);
static json_type JExprMemberAccess(const ast::MemberAccessExpr& member_expr);
static json_type JExprCall(const ast::CallExpr& call_expr);

static json_type JExprBase(const ast::ExprBase& base_expr)
{
    // std::cout << "JExprBase" << std::endl;
    switch (base_expr.kind)
    {
    case ast::ExprBase::Identifier:
        return JExprIdentifier(static_cast<const ast::IdentifierExpr&>(base_expr));
    case ast::ExprBase::MemberAccess:
        return JExprMemberAccess(static_cast<const ast::MemberAccessExpr&>(base_expr));
    case ast::ExprBase::Call:
        return JExprCall(static_cast<const ast::CallExpr&>(base_expr));
        
    default:
    case ast::ExprBase::Unknown:;
    }
    
    return json_type{
        {"e_kind", EKindToString(base_expr.kind)},
        {"text", base_expr.text},
    };    
}

static json_type JExprIdentifier(const ast::IdentifierExpr& ident_expr)
{
    return json_type{
        {"e_kind", EKindToString(ident_expr.kind)},
        {"name", ident_expr.name}
    };
}

static json_type JExprMemberAccess(const ast::MemberAccessExpr& member_expr)
{
    return json_type{
        {"e_kind", EKindToString(member_expr.kind)},
        {"base", member_expr.base ? JExprBase(*member_expr.base) : json_type{nullptr}},
        {"member_name", member_expr.member_name},
    };
}

static json_type JExprCall(const ast::CallExpr& call_expr)
{
    json_type j_call;

    // callee
    j_call["e_kind"] = EKindToString(call_expr.kind);
    j_call["callee"] = call_expr.callee ? JExprBase(*call_expr.callee) : json_type{nullptr};
    
    // args collection
    json_type args = json_type::array();
    for (const auto& arg : call_expr.args)
    {
        args.push_back(arg ? JExprBase(*arg) : json_type{nullptr});
    }
    j_call["args"] = std::move(args);

    return j_call;
}

static json_type JExprStmt(const ast::ExprStmt& expr_stmt)
{
    json_type j_stmt = json_type{
        {"kind", KindToString(expr_stmt.kind_)},
    };

    if (expr_stmt.expr == nullptr)
    {
        j_stmt["expr"] = expr_stmt.text;
        return j_stmt;
    }
    
    j_stmt["expr"] = JExprBase(*expr_stmt.expr);
    j_stmt["text"] = expr_stmt.text;
    
    return j_stmt;
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

static json_type JFuncPapams(const std::vector<ast::Param>& params)
{
    json_type j_params = json_type::array();
    for (const auto& param : params)
    {
        json_type j_param = json_type{
            {"external_name", param.external_name},
            {"local_name", param.local_name},
            {"type", param.type_name},
        };

        j_params.push_back(std::move(j_param));
    }

    return j_params;
}

static json_type JSignature(const ast::FunctionSignature& sig)
{
    return json_type{
        {"name", sig.name},
        {"params", JFuncPapams(sig.params)},
        {"return_type", sig.return_type},
    };
}

static json_type JFunctionDeclStmt(const ast::FunctionDeclStmt& func_stmt)
{
    return json_type{
        {"kind", KindToString(func_stmt.kind_)},
        {"signature", JSignature(func_stmt.signature)},
        {"body", func_stmt.body ? JBlockStmt(*func_stmt.body) : json_type(nullptr)},
        {"is_initializer", func_stmt.is_initializer},
        {"is_deinitializer", func_stmt.is_deinitializer},
        {"is_static", func_stmt.is_static},
        {"is_override", func_stmt.is_override},
    };
}

static json_type JClassDeclStmt(const ast::ClassDeclStmt& class_stmt)
{
    json_type j_class;
    // name and base name of the class
    j_class["kind"] = KindToString(class_stmt.kind_);
    j_class["name"] = class_stmt.name;
    j_class["base_name"] = class_stmt.base_class_name;

    // collecting members
    json_type members = json_type::array();
    for (const auto& member : class_stmt.members)
    {
        members.push_back(member ? JStmt(*member) : json_type{nullptr});
    }
    j_class["members"] = std::move(members);

    return j_class;
}

static json_type JVarDeclStmt(const ast::VarDeclStmt& vardecl_stmt)
{
    // std::cout << "JVarDeclStmt" << std::endl;
    return json_type{
        {"kind" , KindToString(vardecl_stmt.kind_)},
        {"name", vardecl_stmt.name},
        {"type", vardecl_stmt.type_name},
        {"init", vardecl_stmt.initializer ? JExprStmt(*vardecl_stmt.initializer) : json_type{nullptr}},
        {"is_property", vardecl_stmt.is_property},
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
    case Kind::Function:    return JFunctionDeclStmt(static_cast<const ast::FunctionDeclStmt&>(stmt));
    case Kind::Class:       return JClassDeclStmt(static_cast<const ast::ClassDeclStmt&>(stmt));
    case Kind::VarDecl:     return JVarDeclStmt(static_cast<const ast::VarDeclStmt&>(stmt));
    default:
        break;
    }
    return {{"kind", "unknown"}};
}

void JsonTreeSerializer::Print(const ast::Stmt& root)
{
    Print(std::cout, root);
}

void JsonTreeSerializer::Print(std::ostream& os, const ast::Stmt& root)
{
    os << JStmt(root).dump(2) << std::endl;
}


// oo_model serialization 
template <class T>
std::vector<T> to_sorted_vector(const std::unordered_set<T>& s) {
    std::vector<T> v;
    v.reserve(s.size());
    for (const auto& x : s) v.push_back(x);
    std::sort(v.begin(), v.end());
    return v;
}

std::vector<std::string> to_sorted_vector(std::vector<std::string> v) {
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
    return v;
}

json_type field_to_json(const pma::metric::FieldInfo& f) {
    json_type j;
    j["name"] = f.name;
    j["type"] = f.type;
    return j;
}

json_type param_to_json(const pma::metric::ParamInfo& p) {
    json_type j;
    j["external_name"] = p.external_name;
    j["local_name"] = p.local_name;
    j["type"] = p.type;
    return j;
}

json_type method_to_json(const pma::metric::MethodInfo& m) {
    json_type j;
    j["name"] = m.name;
    j["isInit"] = m.isInit;
    j["isDeinit"] = m.isDeinit;

    // params
    {
        json_type params = json_type::array();
        for (const auto& p : m.params)
        {
            params.push_back(param_to_json(p));
        }
        j["params"] = std::move(params);
    }

    j["returnType"] = m.returnType;

    // sets (sorted for stable output)
    std::cout << m.usedFields.size() << " " << m.calledMethods.size() << " " << m.referencedTypes.size() << std::endl;
    j["usedFields"] = to_sorted_vector(m.usedFields);
    j["calledMethods"] = to_sorted_vector(m.calledMethods);
    j["referencedTypes"] = to_sorted_vector(m.referencedTypes);

    return j;
}

json_type class_to_json(const pma::metric::ClassInfo& c) {
    json_type j;
    j["name"] = c.name;
    j["base"] = c.base;

    // fields
    {
        json_type fields = json_type::array();
        for (const auto& f : c.fields) 
        {
            fields.push_back(field_to_json(f));
        }
        j["fields"] = std::move(fields);
    }

    // methods
    {
        json_type methods = json_type::array();
        
        for (const auto& m : c.methods) 
        {
            methods.push_back(method_to_json(m));
        }
        j["methods"] = std::move(methods);
    }

    // children (sorted, unique)
    j["children"] = to_sorted_vector(c.children);

    return j;
}

void JsonTreeSerializer::Print(std::ostream& os, const metric::OOModel& model) 
{
    json_type root;

    // classes: stable ordering by class name
    std::vector<std::string> classNames;
    classNames.reserve(model.classes.size());
    for (const auto& [name, _] : model.classes) classNames.push_back(name);
    std::sort(classNames.begin(), classNames.end());

    json_type classes = json_type::array();

    for (const auto& name : classNames) 
    {
        auto it = model.classes.find(name);
        if (it == model.classes.end()) continue;

        // serializing ClassInfo
        classes.push_back(class_to_json(it->second));
    }

    root["classes"] = std::move(classes);

    // pretty print
    os << root.dump(2);
}

json_type JMetric(const metric::CKMetric& metric)
{
    return json_type{
        {"wmc", metric.wmc},
        {"dit", metric.dit},
        {"noc", metric.noc},
        {"cbo", metric.cbo},
        {"rfc", metric.rfc},
        {"lcom", metric.lcom},
        
    };
}

void JsonTreeSerializer::Print(std::ostream& os, const std::unordered_map<std::string, metric::CKMetric>& metrics)
{
    json_type j_metrics = json_type::array();

    for (const auto& [name, metric] : metrics)
    {
        json_type j_metric;
        j_metric[name] = JMetric(metric);

        j_metrics.push_back(std::move(j_metric));
    }

    os << j_metrics.dump(2);
}

} // namespace pma::printers
