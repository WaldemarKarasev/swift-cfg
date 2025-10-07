#pragma once

// std
#include <string>

namespace pma
{
    
class App
{
public:
    int BuildCfg(int argc, char** argv);
    int BuildCfg(std::string source_file, std::string out_ast_file, std::string out_dot_file);
};

} // namespace pma
