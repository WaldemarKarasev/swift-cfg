#pragma once

// pma
#include <frontend/ast_builder.hpp>
#include <frontend/registry.hpp>

namespace pma::frontends::swift
{

class SwiftTreeSitterAstBuilder : public IAstBuilder
{
public:
    virtual std::unique_ptr<ast::BlockStmt> BuildFromRoot(const utils::SourceView& source_code, pma::diagnostic::IDiagnostic& diag) override;  
};

std::unique_ptr<IAstBuilder> MakeSwiftTreeSitterAstBuilder()
{
    return std::make_unique<SwiftTreeSitterAstBuilder>();
}

// static frontend registration
static bool register_frontend = [](){
    frontends::registry::AutoRegister reg(registry::Lang::Swift, registry::Frontend::TreeSitter, &MakeSwiftTreeSitterAstBuilder);
    return true;
}();

} // namespace pma::frontends::swift



