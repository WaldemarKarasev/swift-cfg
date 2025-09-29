#pragma once

// std
#include <optional>
#include <string>

// pma
#include <utils/source_range.hpp>

namespace pma::diagnostic
{

class IDiagnostic
{
public:
    using SourceRangeType = std::optional<utils::SourceRange>; 

public:
    enum class Severity : int
    {
        Info,
        Warning,
        Error,
    };

    static std::string ToString(Severity s) 
    {
        switch (s)
        {
        case Severity::Info:    return "Info";
        case Severity::Warning: return "Warning";
        case Severity::Error:   return "Error";
        }
        return {};
    }

public:
    virtual ~IDiagnostic() = default;
    void error(std::string msg, SourceRangeType src_rng = std::nullopt) { Report(Severity::Error,   std::move(msg), src_rng); }
    void warn (std::string msg, SourceRangeType src_rng = std::nullopt) { Report(Severity::Warning, std::move(msg), src_rng); }
    void info (std::string msg, SourceRangeType src_rng = std::nullopt) { Report(Severity::Info,    std::move(msg), src_rng); }

private:
    virtual void Report(Severity s, std::string msg, SourceRangeType src_rng) = 0;
};

class StdOutDiagnostic : public IDiagnostic
{
public:
    StdOutDiagnostic(utils::SourceView source);

    virtual void Report(Severity s, std::string msg, SourceRangeType src_rng) override;

private:
    utils::SourceView source_;
};

} // namespace pma
