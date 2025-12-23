#pragma once

#include "oo_model.hpp"
#include <ast/ast.hpp>

namespace pma::metric {

class OOModelBuilder 
{
public:
    using TypeEnv = std::unordered_map<std::string, std::string>;

public:
    OOModel Build(const ast::BlockStmt& root);

private:
    void VisitStmt(const ast::Stmt& s);

private:
    void BuildClass(const ast::Stmt& s);
    void BuildField(ClassInfo& ci, const ast::Stmt& member);
    void BuildMethod(ClassInfo& ci, const ast::Stmt& member);

    void BuildInheritanceEdges(); // updating all children

private:
    void CollectExtraData(MethodInfo& mi, const ast::FunctionDeclStmt& fn, const ClassInfo& owner, TypeEnv& env);
    void CollectExtraDataFromStmt(MethodInfo& mi, const ast::Stmt& s, const ClassInfo& owner, TypeEnv& env);

private:
    // Collecting type used in method
    void CollectReferencedTypesFromExpr(MethodInfo& mi, const ast::ExprBase& expr);
    void CollectReferencedTypesFromText(MethodInfo& mi, const std::string& text);

private:
    void CollectCalledMethodsFromExpr(MethodInfo& mi, const ast::ExprBase& expr);
    void CollectCalledMethodsFromText(MethodInfo& mi, const std::string& text);

private:
    void CollectUsedFieldsFromExpr(MethodInfo& mi, const ast::ExprBase& expr, const ClassInfo& owner);
    void CollectUsedFieldsFromText(MethodInfo& mi, const std::string& text, const ClassInfo& owner);

private:
    OOModel model_;
};

} // namespace pma::metric
