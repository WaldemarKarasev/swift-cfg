#include "ck_metric_calculator.hpp"

#include <iostream>
#include <algorithm>

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

bool CKMetricsCalculator::IsExcludedType(const std::string& t) const
{
    return t.empty() || cfg_.cbo_excluded_types.find(t) != cfg_.cbo_excluded_types.end();
}

std::string CKMetricsCalculator::ExtractReceiver(const std::string& call)
{
    // expecting "Receiver.method"
    const auto dot = call.find('.');
    if (dot == std::string::npos || dot == 0) return {};

    // trim receiver
    size_t b = 0;
    while (b < dot && std::isspace(static_cast<unsigned char>(call[b]))) ++b;

    size_t e = dot;
    while (e > b && std::isspace(static_cast<unsigned char>(call[e - 1]))) --e;

    if (e <= b) return {};
    return call.substr(b, e - b);
}


std::string Trim(std::string s) {
    auto is_ws = [](unsigned char c) { return std::isspace(c) != 0; };
    while (!s.empty() && is_ws((unsigned char)s.front())) s.erase(s.begin());
    while (!s.empty() && is_ws((unsigned char)s.back())) s.pop_back();
    return s;
}

void StripAllSpaces(std::string& s) {
    s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
    }), s.end());
}

std::string NormalizeType(std::string t) {
    t = Trim(std::move(t));
    StripAllSpaces(t);

    // Remove optional suffixes
    while (!t.empty() && (t.back() == '?' || t.back() == '!')) t.pop_back();

    // [T] -> T
    if (t.size() >= 2 && t.front() == '[' && t.back() == ']') {
        t = t.substr(1, t.size() - 2);
    }

    // Optional<T>, Array<T>, Set<T>, Dictionary<K,V>
    // Returning internal string inside of <> (or K,V for Dictionary)
    auto lt = t.find('<');
    auto gt = t.rfind('>');
    if (lt != std::string::npos && gt != std::string::npos && lt < gt) {
        std::string inner = t.substr(lt + 1, gt - lt - 1);
        // inner can be K,V or T
        return inner;
    }

    return t;
}
void CKMetricsCalculator::AddTypeTokens(TypeSet& out, const std::string& typeExpr) const
{
    for (size_t i = 0; i < typeExpr.size();)
    {
        unsigned char c = static_cast<unsigned char>(typeExpr[i]);
        if (std::isalpha(c) || typeExpr[i] == '_')
        {
            size_t j = i + 1;
            while (j < typeExpr.size())
            {
                unsigned char cc = static_cast<unsigned char>(typeExpr[j]);
                if (std::isalnum(cc) || typeExpr[j] == '_') ++j;
                else break;
            }

            std::string tok = typeExpr.substr(i, j - i);
            if (!tok.empty()
                && std::isupper(static_cast<unsigned char>(tok[0]))
                && !IsExcludedType(tok))
            {
                out.insert(std::move(tok));
            }
            i = j;
        }
        else
        {
            ++i;
        }
    }
}
CKMetricsCalculator::TypeSet 
CKMetricsCalculator::ComputeCBOTypeSetFor(const OOModel& model
                                        , const std::string& className
                                        , std::unordered_map<std::string, TypeSet>& cache) const
{
    // std::cout << "= name: " << className << std::endl;
    // 1) cache hit
    if (auto it = cache.find(className); it != cache.end()) return it->second;

    TypeSet coupled;

    auto itC = model.classes.find(className);
    if (itC == model.classes.end())
    {
        // External class : empty
        cache.emplace(className, TypeSet{});
        return TypeSet{};
    }

    const ClassInfo& cls = itC->second;

    // ---- DirectTypes(C) ----

    // (a) inheritance relation itself    
    if (!cls.base.empty()) AddTypeTokens(coupled, cls.base);
    
    // (b) fields
    for (const auto& f : cls.fields)
    {
        AddTypeTokens(coupled, f.type);   
    }

    // (c) methods
    for (const auto& m : cls.methods)
    {
        for (const auto& p : m.params) 
        {
            AddTypeTokens(coupled, p.type);
        }
        
        AddTypeTokens(coupled, m.returnType);

        if (true || cfg_.cbo_use_method_referenced_types)
        {
            for (const auto& t : m.referencedTypes)
            {
                AddTypeTokens(coupled, t);
            }
        }
    }

    #if 0
    std::cout << "-----------------" << std::endl;
    for (const auto& type : coupled)
    {
        std::cout << "type: " << type << std::endl;
    }
    #endif

    // ---- AllTypes(C) = DirectTypes(C) ∪ AllTypes(Base(C)) ----
    if (not cls.base.empty())
    {
        if (model.classes.contains(cls.base))
        {
            TypeSet baseSet = ComputeCBOTypeSetFor(model, cls.base, cache);
            coupled.insert(baseSet.begin(), baseSet.end());
        }
    }

    coupled.erase(className); // self-coupling
    cache.emplace(className, coupled);
    return coupled;
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

    auto wmc_metric  = ComputeWMC(model);
    auto dit_metric  = ComputeDIT(model);
    auto noc_metric  = ComputeNOC(model);
    auto cbo_metric  = ComputeCBO(model);
    auto rfc_metric  = ComputeRFC(model);
    auto lcom_metric = ComputeLCOM(model);


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

        if (cbo_metric.contains(name))
        {
            metric.cbo = cbo_metric.at(name);
        }

        if (rfc_metric.contains(name))
        {
            metric.rfc = rfc_metric.at(name);
        }

        if (lcom_metric.contains(name))
        {
            metric.lcom = lcom_metric.at(name);
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

// CBO
std::unordered_map<std::string, int> CKMetricsCalculator::ComputeCBO(const OOModel& model) const
{
    std::unordered_map<std::string, TypeSet> cache;
    cache.reserve(model.classes.size());

    // Lazy filling cache with referenced types
    for (const auto& [name, _] : model.classes)
    {
        (void)ComputeCBOTypeSetFor(model, name, cache);
    }

    #if 1
    std::cout << "=========== CBO Types CACHE ===========" << std::endl;
    
    for (const auto& [name, types] : cache)
    {
        std::cout << "------ class: " << name << " ------" << std::endl;
        for (const auto& type : types)
        {
            std::cout << "type: " << type << std::endl;
        }
    }
    #endif

    std::unordered_map<std::string, int> out;
    out.reserve(model.classes.size());

    for (const auto& [name, _] : model.classes)
    {
        auto it = cache.find(name);
        out.emplace(name, it == cache.end() ? 0 : static_cast<int>(it->second.size()));
    }

    return out;
}

// RFC
std::unordered_map<std::string, int> CKMetricsCalculator::ComputeRFC(const OOModel& model) const
{
    std::unordered_map<std::string, int> rfc;
    rfc.reserve(model.classes.size());

    for (const auto& [className, cls] : model.classes)
    {
        // M - number of class methods (like in WMC cfg_.count_init/count_deinit)
        const int M = CountMethodsDeclared(cls);

        // R - unique call for other classes methods
        std::unordered_set<std::string> externalCalls;
        externalCalls.reserve(32);

        for (const auto& m : cls.methods)
        {
            for (const auto& call : m.calledMethods)
            {
                // call usually Receiver.method
                const std::string receiver = ExtractReceiver(call);
                if (receiver.empty()) continue;

                // doesn't counting internall calls
                if (receiver == "self" || receiver == "Self") continue;
                if (receiver == className) continue;

                // super.foo 
                externalCalls.insert(call);
            }
        }

        const int R = static_cast<int>(externalCalls.size());
        rfc.emplace(className, M + R);
    }

    return rfc;
}

std::unordered_map<std::string, int> CKMetricsCalculator::ComputeLCOM(const OOModel& model) const
{
    std::unordered_map<std::string, int> out;
    out.reserve(model.classes.size());

    for (const auto& [className, cls] : model.classes)
    {
        // Callecting method list for lcom metric
        std::vector<const MethodInfo*> methods;
        methods.reserve(cls.methods.size());

        for (const auto& m : cls.methods)
        {
            if (!cfg_.count_init && m.isInit) continue;
            if (!cfg_.count_deinit && m.isDeinit) continue;
            methods.push_back(&m);
        }

        int CM = 0;
        int UCM = 0;

        for (size_t i = 0; i < methods.size(); ++i)
        {
            for (size_t j = i + 1; j < methods.size(); ++j)
            {
                const auto& a = methods[i]->usedFields;
                const auto& b = methods[j]->usedFields;

                bool share = false;
                // itereting through smaller set
                if (a.size() <= b.size())
                {
                    for (const auto& f : a) { if (b.contains(f)) { share = true; break; } }
                }
                else
                {
                    for (const auto& f : b) { if (a.contains(f)) { share = true; break; } }
                }

                if (share) ++CM;
                else ++UCM;
            }
        }

        const int lcom = std::max(UCM - CM, 0);
        out.emplace(className, lcom);
    }

    return out;
}


} // namespace pma::metric
