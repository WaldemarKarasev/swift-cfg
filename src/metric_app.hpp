#pragma once

// std
#include <string>

namespace pma
{
    
class MetricApp
{
public:
    int Count(int argc, char** argv);
    int Count(std::string source_file, std::string out_ast_file, std::string out_json_metric);
};

} // namespace pma
