#pragma once

#include "oo_model.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace pma::metric {

struct CKMetric
{
    float wmc = 0;
    float dit = 0;
    float noc = 0;
    float cbo = 0;
    float rfc = 0;
    float lcom = 0;
};

struct AllMetrics
{
    CKMetric average;
    std::unordered_map<std::string, CKMetric> metrics;
};

// Chidamber & Kemerer - CK metrics
class CKMetricsCalculator 
{
public:
    using TypeSet = std::unordered_set<std::string>;
    struct Config 
    {
        // of/off counting an init functions into WMC
        bool count_init = false;
        // of/off counting an deinit functions into WMC
        bool count_deinit = false;

        bool cbo_use_method_referenced_types = false;
        // Excluding table for cbo calculation
        std::unordered_set<std::string> cbo_excluded_types = {
            "Int", "Int8", "Int16", "Int32", "Int64",
            "UInt", "UInt8", "UInt16", "UInt32", "UInt64",
            "Float", "Double",
            "Bool",
            "String", "Character",
            "Void",
            "Any", "AnyObject",
            "Never",
        };
    };

    using Metric = CKMetric;

    CKMetricsCalculator() = default;
    explicit CKMetricsCalculator(Config cfg) : cfg_(cfg) {}

    // calculation all metrics
    AllMetrics Count(const OOModel& model);

    // WMC
    std::unordered_map<std::string, int> ComputeWMC(const OOModel& model) const;

    // DIT
    std::unordered_map<std::string, int> ComputeDIT(const OOModel& model) const;

    // NOC
    std::unordered_map<std::string, int> ComputeNOC(const OOModel& model) const;

    // CBO
    std::unordered_map<std::string, int> ComputeCBO(const OOModel& model) const;

    // RFC
    std::unordered_map<std::string, int> ComputeRFC(const OOModel& model) const;

    // LCOM
    std::unordered_map<std::string, int> ComputeLCOM(const OOModel& model) const;


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

    // CBO helpers
    void AddTypeTokens(TypeSet& out, const std::string& typeExpr) const;
    TypeSet ComputeCBOTypeSetFor(const OOModel& model
                                , const std::string& className
                                , std::unordered_map<std::string, TypeSet>& cache) const;
    bool IsExcludedType(const std::string& t) const;

private:
    static std::string ExtractReceiver(const std::string& call);


private:
    Config cfg_;
};

} // namespace pma::metric
