#include "builder.hpp"

#include <iostream>
#include <regex>

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

    // fields
    for (const auto& m : cls.members) 
    {
        if (m == nullptr) continue;
        if (m->kind_ == ast::Stmt::VarDecl) 
        {
            BuildField(ci, *m);
        }
    }

    // methods
    for (const auto& m : cls.members) 
    {
        if (m == nullptr) continue;
        if (m->kind_ == ast::Stmt::Function) 
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

    TypeEnv env;
    CollectExtraData(mi, fn, ci, env);

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


void OOModelBuilder::CollectExtraData(MethodInfo& mi, const ast::FunctionDeclStmt& fn, const ClassInfo& owner, TypeEnv& env)
{
    if (!fn.body) return;
    CollectExtraDataFromStmt(mi, *fn.body, owner, env);
}

void OOModelBuilder::CollectExtraDataFromStmt(MethodInfo& mi, const ast::Stmt& s, const ClassInfo& owner, TypeEnv& env)
{
    switch (s.kind_)
    {
        case ast::Stmt::Block:
        {
            const auto& b = static_cast<const ast::BlockStmt&>(s);
            for (const auto& st : b.stmts_)
            {
                if (st)
                {
                    CollectExtraDataFromStmt(mi, *st, owner, env);
                } 
            }
            break;
        }

        case ast::Stmt::If:
        {
            const auto& iff = static_cast<const ast::IfStmt&>(s);
            CollectReferencedTypesFromText(mi, iff.cond);
            CollectCalledMethodsFromText(mi, iff.cond);
            CollectUsedFieldsFromText(mi, iff.cond, owner);
            if (iff.thenB) CollectExtraDataFromStmt(mi, *iff.thenB, owner, env);
            if (iff.elseB) CollectExtraDataFromStmt(mi, *iff.elseB, owner, env);
            break;
        }

        case ast::Stmt::While:
        {
            const auto& w = static_cast<const ast::WhileStmt&>(s);
            CollectReferencedTypesFromText(mi, w.cond);
            CollectUsedFieldsFromText(mi, w.cond, owner);
            CollectCalledMethodsFromText(mi, w.cond);

            if (w.body) CollectExtraDataFromStmt(mi, *w.body, owner, env);
            break;
        }

        case ast::Stmt::DoWhile:
        {
            const auto& d = static_cast<const ast::DoWhileStmt&>(s);
            CollectReferencedTypesFromText(mi, d.cond);
            CollectUsedFieldsFromText(mi, d.cond, owner);
            CollectCalledMethodsFromText(mi, d.cond);
            
            if (d.body) CollectExtraDataFromStmt(mi, *d.body, owner, env);
            break;
        }

        case ast::Stmt::For:
        {
            const auto& f = static_cast<const ast::ForStmt&>(s);
            CollectReferencedTypesFromText(mi, f.collection);
            
            CollectUsedFieldsFromText(mi, f.item, owner);
            CollectUsedFieldsFromText(mi, f.collection, owner);
            

            CollectCalledMethodsFromText(mi, f.item);
            CollectCalledMethodsFromText(mi, f.collection);

            if (f.body) CollectExtraDataFromStmt(mi, *f.body, owner, env);
            break;
        }

        case ast::Stmt::Expr:
        {
            const auto& e = static_cast<const ast::ExprStmt&>(s);
            if (e.expr)
            {
                CollectReferencedTypesFromExpr(mi, *e.expr);
                CollectCalledMethodsFromExpr(mi, *e.expr);
                CollectUsedFieldsFromExpr(mi, *e.expr, owner);
            }

            break;
        }

        case ast::Stmt::VarDecl:
        {
            const auto& v = static_cast<const ast::VarDeclStmt&>(s);
            if (v.initializer)
            {
                CollectReferencedTypesFromText(mi, v.initializer->text);
                if (v.initializer->expr) 
                {
                    CollectReferencedTypesFromText(mi, v.initializer->expr->text);
                }
            }
            break;
        }
        case ast::Stmt::Return:
        {
            const auto& r = static_cast<const ast::ReturnStmt&>(s);
            CollectUsedFieldsFromText(mi, r.lable, owner);
            CollectCalledMethodsFromText(mi, r.lable);
            break;
        }

        default:
            break;
    }
}

static bool LooksLikeTypeName(const std::string& s)
{
    return !s.empty() && std::isupper(static_cast<unsigned char>(s[0]));
}

static bool IsSelfLike(const std::string& s)
{
    return s == "self" || s == "Self" || s == "super";
}

static bool IsFieldOf(const ClassInfo& owner, const std::string& name)
{
    for (const auto& f : owner.fields)
        if (f.name == name) return true;
    return false;
}


void OOModelBuilder::CollectReferencedTypesFromExpr(MethodInfo& mi, const ast::ExprBase& expr)
{
    using K = ast::ExprBase::Kind; // или как у тебя называется enum

    switch (expr.kind)
    {
        case K::Identifier:
        {
            const auto& id = static_cast<const ast::IdentifierExpr&>(expr);
            if (LooksLikeTypeName(id.name))
                mi.referencedTypes.insert(id.name);
            break;
        }

        case K::MemberAccess:
        {
            const auto& ma = static_cast<const ast::MemberAccessExpr&>(expr);

            // colleecting base
            if (ma.base) CollectReferencedTypesFromExpr(mi, *ma.base);

            // if base - Identifier then  TypeName.member
            if (ma.base && ma.base->kind == K::Identifier)
            {
                const auto& bid = static_cast<const ast::IdentifierExpr&>(*ma.base);
                if (LooksLikeTypeName(bid.name))
                    mi.referencedTypes.insert(bid.name);

                // optional, if member_name looks like type (Foo.Bar)
                if (LooksLikeTypeName(ma.member_name))
                    mi.referencedTypes.insert(ma.member_name);
            }
            break;
        }

        case K::Call:
        {
            const auto& call = static_cast<const ast::CallExpr&>(expr);

            // callee can be Identifier or MemberAccess
            if (call.callee)
                CollectReferencedTypesFromExpr(mi, *call.callee);

            // args
            for (const auto& a : call.args)
                if (a) CollectReferencedTypesFromExpr(mi, *a);

            // if callee == Identifier —> ctor or call with type
            if (call.callee && call.callee->kind == K::Identifier)
            {
                const auto& cid = static_cast<const ast::IdentifierExpr&>(*call.callee);
                if (LooksLikeTypeName(cid.name))
                    mi.referencedTypes.insert(cid.name);
            }

            // if callee == MemberAccess, then:
            // Foo.init(...) => Foo (type)
            // Foo.bar(...)  => Foo (type)
            if (call.callee && call.callee->kind == K::MemberAccess)
            {
                const auto& ma = static_cast<const ast::MemberAccessExpr&>(*call.callee);
                if (ma.base && ma.base->kind == K::Identifier)
                {
                    const auto& bid = static_cast<const ast::IdentifierExpr&>(*ma.base);
                    if (LooksLikeTypeName(bid.name))
                        mi.referencedTypes.insert(bid.name);
                }
            }

            break;
        }

        case K::Unknown:
        default:
            CollectReferencedTypesFromText(mi, expr.text);
            break;
    }
}


void OOModelBuilder::CollectReferencedTypesFromText(MethodInfo& mi, const std::string& text)
{
    // std::cout << "CollectReferencedTypesFromText text: " << text << std::endl;
    if (text.empty()) return;

    // Identifier should started with high register
    // Foo, URLSession, MyType2
    static const std::regex kTypeIdent(R"(\b([A-Z][A-Za-z0-9_]*)\b)");

    for (auto it = std::sregex_iterator(text.begin(), text.end(), kTypeIdent);
         it != std::sregex_iterator();
         ++it)
    {
        std::string name = (*it)[1].str();
        mi.referencedTypes.insert(std::move(name));
    }
}

void OOModelBuilder::CollectCalledMethodsFromExpr(MethodInfo& mi, const ast::ExprBase& expr)
{
    using K = ast::ExprBase::Kind;

    switch (expr.kind)
    {
        case K::Call:
        {
            const auto& call = static_cast<const ast::CallExpr&>(expr);

            // call processing
            if (call.callee)
            {
                if (call.callee->kind == K::MemberAccess)
                {
                    const auto& ma = static_cast<const ast::MemberAccessExpr&>(*call.callee);

                    // receiver
                    std::string receiver = "<?>"; // unknown type

                    if (ma.base && ma.base->kind == K::Identifier)
                    {
                        const auto& bid = static_cast<const ast::IdentifierExpr&>(*ma.base);
                        receiver = bid.name; // self / super / obj / TypeName
                    }

                    // method
                    const std::string& method = ma.member_name;

                    mi.calledMethods.insert(receiver + "." + method);

                    // ТCollecting from base
                    if (ma.base) CollectCalledMethodsFromExpr(mi, *ma.base);
                }
                else if (call.callee->kind == K::Identifier)
                {
                    const auto& id = static_cast<const ast::IdentifierExpr&>(*call.callee);
                    //  call method(...)
                    mi.calledMethods.insert(std::string("self.") + id.name);
                }
                else
                {
                    CollectCalledMethodsFromExpr(mi, *call.callee);
                }
            }

            // args
            for (const auto& a : call.args)
                if (a) CollectCalledMethodsFromExpr(mi, *a);

            break;
        }

        case K::MemberAccess:
        {
            const auto& ma = static_cast<const ast::MemberAccessExpr&>(expr);
            if (ma.base) CollectCalledMethodsFromExpr(mi, *ma.base);
            break;
        }

        case K::Identifier:
            break;

        case K::Unknown:
        default:
            // fallback
            CollectCalledMethodsFromText(mi, expr.text);
            break;
    }
}

void OOModelBuilder::CollectCalledMethodsFromText(MethodInfo& mi, const std::string& text)
{
    // std::cout << "CollectCalledMethodsFromText text: " << text << std::endl;
    if (text.empty()) return;

    // Examples:
    //   Foo.bar(...)
    //   obj.baz(...)
    //   super.viewDidLoad(...)
    static const std::regex kCall(R"(\b([A-Za-z_][A-Za-z0-9_]*)\s*\.\s*([A-Za-z_][A-Za-z0-9_]*)\s*\()");

    for (auto it = std::sregex_iterator(text.begin(), text.end(), kCall);
         it != std::sregex_iterator();
         ++it)
    {
        const std::string receiver = (*it)[1].str();
        const std::string method   = (*it)[2].str();
        mi.calledMethods.insert(receiver + "." + method);
    }
}

void OOModelBuilder::CollectUsedFieldsFromExpr(MethodInfo& mi,
                                               const ast::ExprBase& expr,
                                               const ClassInfo& owner)
{
    using K = ast::ExprBase::Kind;

    switch (expr.kind)
    {
        case K::Identifier:
        {
            const auto& id = static_cast<const ast::IdentifierExpr&>(expr);

            // This identifier is field if it is a class field
            if (IsFieldOf(owner, id.name))
                mi.usedFields.insert(id.name);

            break;
        }

        case K::MemberAccess:
        {
            const auto& ma = static_cast<const ast::MemberAccessExpr&>(expr);

            // self.x / super.x
            if (ma.base && ma.base->kind == K::Identifier)
            {
                const auto& bid = static_cast<const ast::IdentifierExpr&>(*ma.base);
                if (IsSelfLike(bid.name))
                {
                    // member_name может быть свойством
                    if (IsFieldOf(owner, ma.member_name))
                        mi.usedFields.insert(ma.member_name);
                    else
                        mi.usedFields.insert(ma.member_name); // если хочешь считать свойства даже без объявления в fields
                }
            }

            // base
            if (ma.base) CollectUsedFieldsFromExpr(mi, *ma.base, owner);

            break;
        }

        case K::Call:
        {
            const auto& call = static_cast<const ast::CallExpr&>(expr);

            // callee can consist: self.x()
            if (call.callee) CollectUsedFieldsFromExpr(mi, *call.callee, owner);

            // args
            for (const auto& a : call.args)
                if (a) CollectUsedFieldsFromExpr(mi, *a, owner);

            break;
        }

        case K::Unknown:
        default:
            // fallback
            CollectUsedFieldsFromText(mi, expr.text, owner);
            break;
    }
}

void OOModelBuilder::CollectUsedFieldsFromText(MethodInfo& mi, const std::string& text, const ClassInfo& owner)
{
    // std::cout << "CollectUsedFieldsFromText text: " << text << std::endl;
    if (text.empty()) return;

    // self.field
    static const std::regex kSelfField(R"(\bself\s*\.\s*([A-Za-z_][A-Za-z0-9_]*)\b)");
    for (auto it = std::sregex_iterator(text.begin(), text.end(), kSelfField);
         it != std::sregex_iterator(); ++it)
    {
        mi.usedFields.insert((*it)[1].str());
    }

    // optional
    static const std::regex kIdent(R"(\b([A-Za-z_][A-Za-z0-9_]*)\b)");
    for (auto it = std::sregex_iterator(text.begin(), text.end(), kIdent);
         it != std::sregex_iterator(); ++it)
    {
        const std::string id = (*it)[1].str();
        // Checking owner field
        for (const auto& f : owner.fields)
        {
            if (f.name == id)
            {
                mi.usedFields.insert(id);
                break;
            }
        }
    }
}

} // namespace pma::metric
