#pragma once

// std
#include <memory>
#include <string>

// pma
#include <ast/ast.hpp>
#include <diagnostic/diagnostic.hpp>
#include <utils/source_range.hpp>

namespace pma::frontends {

struct IAstBuilder 
{
    virtual ~IAstBuilder() = default;
    virtual std::unique_ptr<ast::BlockStmt> BuildFromRoot(const utils::SourceView& source_code, pma::diagnostic::IDiagnostic& diag) = 0;  
};

} // namespace pma::frontends
