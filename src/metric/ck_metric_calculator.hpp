#pragma once

#include "oo_model.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace pma::metric {

struct CKMetric
{
    int wmc = 0;
    int dit = 0;
    int noc = 0;
    int cbo = 0;
    int rfc = 0;
    int lcom = 0;
};

// Chidamber & Kemerer - CK metrics
class CKMetricsCalculator 
{
public:
    struct Config 
    {
        // of/off counting an init functions into WMC
        bool count_init = false;
        // of/off counting an deinit functions into WMC
        bool count_deinit = false;
    };

    using Metric = CKMetric;

    CKMetricsCalculator() = default;
    explicit CKMetricsCalculator(Config cfg) : cfg_(cfg) {}

    // calculation all metrics
    std::unordered_map<std::string, Metric> Count(const OOModel& model);

    // WMC
    std::unordered_map<std::string, int> ComputeWMC(const OOModel& model) const;

    // DIT
    std::unordered_map<std::string, int> ComputeDIT(const OOModel& model) const;

    // NOC
    std::unordered_map<std::string, int> ComputeNOC(const OOModel& model) const;

private:
    // WMC helpers
    int ComputeWMCFor(const OOModel& model,
                      const std::string& className,
                      std::unordered_map<std::string, int>& cache) const;

    int CountMethodsDeclared(const ClassInfo& cls) const;

    // DIT helpers
    int ComputeDITFor(const OOModel& model,
                      const std::string& className,
                      std::unordered_map<std::string, int>& cache) const;


private:
    Config cfg_;
};

} // namespace pma::metric
