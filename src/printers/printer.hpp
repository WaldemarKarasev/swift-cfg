#pragma once

// std
#include <filesystem>
#include <memory>

// pma
#include <cfg/cfg.hpp>
#include <ast/ast.hpp>

namespace pma::printers
{

class IPrinter
{
public:
    virtual ~IPrinter() = default;
    virtual void Print(std::filesystem::path path, const cfg::Graph& cfg) = 0;
};

class ITreeSerializer
{
public:
    virtual ~ITreeSerializer() = default;
    virtual void Print(const ast::Stmt& root) = 0;
    virtual void Print(std::ostream& os, const ast::Stmt& root) = 0;
};

} // namespace pma::printers
