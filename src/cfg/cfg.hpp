#pragma once

// std
#include <string>
#include <vector>

namespace pma::cfg
{
    
struct Edge 
{
    int to{-1};
    std::string label; // "true"/"false"/"fallthrough"/""/etc.
};

struct BasicBlock
{
    int id{-1};
    std::vector<std::string> instrs;
    std::vector<Edge> outs;
    bool closed{false}; // return/throw and etc.
};

struct Graph 
{
    int entry{-1}, exit{-1};
    std::vector<BasicBlock> blocks;
};

} // namespace pma::cfg
