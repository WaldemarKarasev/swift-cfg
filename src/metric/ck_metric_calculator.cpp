#include "ck_metric_calculator.hpp"

namespace pma::metric {

int CKMetricsCalculator::CountMethodsDeclared(const ClassInfo& cls) const 
{
    int count = 0;
    for (const auto& m : cls.methods) 
    {
        if (m.isInit && !cfg_.count_init) continue;
        if (m.isDeinit && !cfg_.count_deinit) continue;
        ++count;
    }
    return count;
}


// DIT and NOC helpers
int CKMetricsCalculator::ComputeDITFor(const OOModel& model
                                    , const std::string& className
                                    , std::unordered_map<std::string, int>& cache) const
{
    if (auto it = cache.find(className); it != cache.end()) return it->second;

    auto itC = model.classes.find(className);
    if (itC == model.classes.end()) 
    {
        cache.emplace(className, 0);
        return 0;
    }

    const ClassInfo& cls = itC->second;

    int dit = 0;
    if (!cls.base.empty()) 
    {
        auto itBase = model.classes.find(cls.base);
        if (itBase == model.classes.end()) 
        {
            // external base class
            dit = 1;
        } 
        else 
        {
            // internal base class 1 + DIT(base)
            dit = 1 + ComputeDITFor(model, cls.base, cache);
        }
    }

    cache.emplace(className, dit);
    return dit;
}

int CKMetricsCalculator::ComputeWMCFor(const OOModel& model,
                                       const std::string& className,
                                       std::unordered_map<std::string, int>& cache) const 
                                       
{
    // cache hit
    if (auto it = cache.find(className); it != cache.end()) 
    {
        return it->second;
    }

    // checking if this is a external class
    auto itC = model.classes.find(className);
    if (itC == model.classes.end()) 
    {
        // for external class WMC == 0
        cache.emplace(className, 0);
        return 0;
    }

    const ClassInfo& cls = itC->second;

    // declared
    int wmc = CountMethodsDeclared(cls);

    // inheritance modes
    if (!cls.base.empty()) 
    {
        wmc += ComputeWMCFor(model, cls.base, cache);
    }

    // save cache
    cache.emplace(className, wmc);
    return wmc;
}

// calculation all metrics
std::unordered_map<std::string, CKMetricsCalculator::Metric> CKMetricsCalculator::Count(const OOModel& model)
{
    std::unordered_map<std::string, CKMetricsCalculator::Metric> metrics;

    auto wmc_metric = ComputeWMC(model);
    auto dit_metric = ComputeDIT(model);
    auto noc_metric = ComputeNOC(model);


    for (const auto& [name, _] : model.classes)
    {
        Metric metric;
        if (wmc_metric.contains(name))
        {
            metric.wmc = wmc_metric.at(name);
        }

        if (dit_metric.contains(name))
        {
            metric.dit = dit_metric.at(name);
        }

        if (noc_metric.contains(name))
        {
            metric.noc = noc_metric.at(name);
        }

        metrics.insert({name, std::move(metric)});
    }

    return metrics;
}

// WMC
std::unordered_map<std::string, int>
CKMetricsCalculator::ComputeWMC(const OOModel& model) const 
{
    std::unordered_map<std::string, int> cache;
    cache.reserve(model.classes.size());

    // Lazy wmc counting
    for (const auto& [name, _] : model.classes) 
    {
        (void)ComputeWMCFor(model, name, cache);
    }

    // filtering external class from collected cache
    std::unordered_map<std::string, int> out;
    out.reserve(model.classes.size());
    for (const auto& [name, _] : model.classes) 
    {
        out.emplace(name, cache[name]);
    }

    return out;
}

// DIT
std::unordered_map<std::string, int> 
CKMetricsCalculator::ComputeDIT(const OOModel& model) const
{
    std::unordered_map<std::string, int> cache;
    cache.reserve(model.classes.size());

    for (const auto& [name, _] : model.classes) 
    {
        (void)ComputeDITFor(model, name, cache);
    }

    std::unordered_map<std::string, int> out;
    out.reserve(model.classes.size());
    for (const auto& [name, _] : model.classes) 
    {
        out.emplace(name, cache[name]);
    }
    return out;
}

// NOC
std::unordered_map<std::string, int> CKMetricsCalculator::ComputeNOC(const OOModel& model) const
{
    std::unordered_map<std::string, int> noc;
    noc.reserve(model.classes.size());

    // Zero initialization because all classes should be presented in report
    for (const auto& [name, _] : model.classes) 
    {
        noc.emplace(name, 0);
    }

    // counting NOC
    for (const auto& [childName, child] : model.classes) 
    {
        if (child.base.empty()) continue;

        auto itParent = model.classes.find(child.base);
        
        // external base class
        if (itParent == model.classes.end()) continue; 

        // child.base
        noc[child.base] += 1;
    }

    return noc;
}

} // namespace pma::metric
