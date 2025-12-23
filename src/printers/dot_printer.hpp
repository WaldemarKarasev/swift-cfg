#pragma once

#include <printers/printer.hpp>

namespace pma::printers
{

class DotPrinter : public IPrinter
{
public:
    virtual void Print(std::ostream& os, const cfg::Graph& cfg) override;
    virtual void Print(std::ostream& os, const ast::Stmt& root) override { return; }
    virtual void Print(std::ostream& os, const metric::OOModel& model) override { return; }
    virtual void Print(std::ostream& os, const metric::AllMetrics& metrics) override { return; }

    // non-inherited function
    virtual void Print(std::filesystem::path path, const cfg::Graph& cfg);
};
    
} // namespace pma::printer
