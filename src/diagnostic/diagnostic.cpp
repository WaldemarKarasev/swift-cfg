#include <diagnostic/diagnostic.hpp>

// std
#include <iostream>
#include <sstream>

namespace pma::diagnostic
{

StdOutDiagnostic::StdOutDiagnostic(utils::SourceView source)
    : source_{source}
{

}


void StdOutDiagnostic::Report(Severity s, std::string msg, SourceRangeType src_rng) 
{
    std::stringstream report;
    report << "[" << IDiagnostic::ToString(s) << "], MSG:" << msg;

    if (src_rng.has_value())
    {
        report << ", CODE:" << source_.slice(src_rng.value());
    }

    std::cout << report.str() << std::endl;
}

} // namespace pma::diagnostic
