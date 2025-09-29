#pragma once

#include <printers/printer.hpp>

namespace pma::printers
{

class DotPrinter : public IPrinter
{
public:
    virtual void Print(std::filesystem::path path, const cfg::Graph& cfg) override;
};
    
} // namespace pma::printer
