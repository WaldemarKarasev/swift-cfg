#pragma once

// std
#include <filesystem>
#include <memory>

// pma
#include <cfg/cfg.hpp>
#include <ast/ast.hpp>
#include <metric/oo_model.hpp>
#include <metric/ck_metric_calculator.hpp>

namespace pma::printers
{

class IPrinter
{
public:
    virtual ~IPrinter() = default;
    virtual void Print(std::ostream& os, const cfg::Graph& cfg) = 0;
    virtual void Print(std::ostream& os, const ast::Stmt& root) = 0;
    virtual void Print(std::ostream& os, const metric::OOModel& model) = 0;
    virtual void Print(std::ostream& os, const std::unordered_map<std::string, metric::CKMetric>& metrics) = 0;
};

} // namespace pma::printers
