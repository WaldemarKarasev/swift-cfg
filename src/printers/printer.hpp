#pragma once

// std
#include <filesystem>

// pma
#include <cfg/cfg.hpp>

namespace pma::printers
{

class IPrinter
{
public:
    virtual ~IPrinter() = default;
    virtual void Print(std::filesystem::path path, const cfg::Graph& cfg) = 0;
};

} // namespace pma::printers
