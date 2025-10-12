#pragma once

// std
#include <string>
#include <vector>

namespace pma::cfg {
    
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
    int entry{-1};
    int exit{-1};
    std::vector<BasicBlock> blocks;

    int NewBlock()
    {
        int id = static_cast<int>(blocks.size());
        blocks.push_back(BasicBlock{});
        blocks.back().id = id;
        return id;
    }

    bool IsBlockClosed(int id) { return blocks[id].closed; }

    void AddInstr(int id, std::string instr)
    {
        blocks[id].instrs.push_back(std::move(instr));
    }

    void AddEdge(int id_from, int id_to, std::string lable)
    {
        blocks[id_from].outs.push_back(Edge{id_to, std::move(lable)});
    }

    void CloseBlock(int id)
    {
        blocks[id].closed = true;
    }
};

} // namespace pma::cfg
