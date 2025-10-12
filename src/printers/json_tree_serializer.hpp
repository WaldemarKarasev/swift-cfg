#pragma once

#include <printers/printer.hpp>

namespace pma::printers
{

class JsonTreeSerializer : public IPrinter
{
public:
    virtual void Print(std::ostream& os, const cfg::Graph& cfg) override;
    virtual void Print(std::ostream& os, const ast::Stmt& root) override;
    void Print(const ast::Stmt& root);
};

} // namespace pma::printers
