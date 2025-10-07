#pragma once

#include <printers/printer.hpp>

namespace pma::printers
{

class JsonTreeSerializer : public ITreeSerializer
{
public:
    virtual void Print(const ast::Stmt& root) override;
    virtual void Print(std::ostream& os, const ast::Stmt& root) override;
};

} // namespace pma::printers
