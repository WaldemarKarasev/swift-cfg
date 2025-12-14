#include "builder.hpp"

#include <iostream>

namespace pma::metric {

OOModel OOModelBuilder::Build(const pma::ast::BlockStmt& root) 
{
    model_ = {};

    // collect all classes and their members
    for (const auto& st : root.stmts_) 
    {
        if (st != nullptr)
        {
            VisitStmt(*st);
        }
    }

    // collecting all inheritance information
    BuildInheritanceEdges();

    // return new model with move operation to clear model_ 
    // state for the next Build function call
    return std::move(model_);
}

void OOModelBuilder::VisitStmt(const pma::ast::Stmt& s) 
{
    switch (s.kind_) 
    {
        case ast::Stmt::Class:
            BuildClass(s);
            break;

        case ast::Stmt::Block: 
        {
            auto& b = static_cast<const ast::BlockStmt&>(s);
            for (const auto& st : b.stmts_) 
            {
                if (st != nullptr) 
                {
                    VisitStmt(*st);
                }
            }
            break;
        }

        default:
            break;
    }
}

void OOModelBuilder::BuildClass(const ast::Stmt& s) 
{
    const auto& cls = static_cast<const ast::ClassDeclStmt&>(s);

    ClassInfo ci;
    ci.name = cls.name;
    ci.base = cls.base_class_name; 

    // fields and methods
    for (const auto& m : cls.members) 
    {
        if (m == nullptr) continue;
        if (m->kind_ == ast::Stmt::VarDecl) 
        {
            BuildField(ci, *m);
        } 
        else if (m->kind_ == ast::Stmt::Function) 
        {
            BuildMethod(ci, *m);
        }
    }

    model_.classes.emplace(ci.name, std::move(ci));
}

void OOModelBuilder::BuildField(ClassInfo& ci, const ast::Stmt& member) 
{
    const auto& v = static_cast<const ast::VarDeclStmt&>(member);

    if (!v.is_property) return;

    FieldInfo fi;
    fi.name = v.name;
    fi.type = v.type_name;

    ci.fields.push_back(std::move(fi));
}

void OOModelBuilder::BuildMethod(ClassInfo& ci, const pma::ast::Stmt& member) {
    const auto& fn = static_cast<const ast::FunctionDeclStmt&>(member);

    MethodInfo mi;
    mi.name = fn.signature.name;

    // init/deinit
    mi.isInit = fn.is_initializer;     
    mi.isDeinit = fn.is_deinitializer;

    // params
    for (const auto& p : fn.signature.params) 
    { 
        ParamInfo info;
        info.external_name = p.external_name;
        info.local_name = p.local_name;
        info.type = p.type_name;
        mi.params.push_back(std::move(info));
    }

    // return type
    mi.returnType = fn.signature.return_type;

    ci.methods.push_back(std::move(mi));
}

void OOModelBuilder::BuildInheritanceEdges() 
{
    // children: parent -> list of direct kids
    for (auto& [childName, child] : model_.classes) 
    {
        if (child.base.empty()) continue;

        auto it = model_.classes.find(child.base);
        
        // external base type
        if (it == model_.classes.end()) continue; 

        it->second.children.push_back(childName);
    }
}

} // namespace pma::metric
