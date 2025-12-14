#pragma once

#include "oo_model.hpp"
#include <ast/ast.hpp>

namespace pma::metric {

class OOModelBuilder 
{
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
    OOModel model_;
};

} // namespace pma::metric
