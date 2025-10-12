#include <printers/dot_printer.hpp>

// std
#include <fstream>
#include <sstream>
#include <iostream>

namespace pma::printers
{

static std::string escape(const std::string& s) {
    std::string out; out.reserve(s.size()+16);
    for (char c : s) {
        if (c=='"' || c=='\\') { out.push_back('\\'); out.push_back(c); }
        else if (c=='\n') { out += "\\l"; } // left-justified line break
        else out.push_back(c);
    }
    return out;
}

void DotPrinter::Print(std::ostream& os, const cfg::Graph& g) 
{
    os << "digraph CFG {\n";
    os << "  node [shape=box, fontname=\"Menlo\", fontsize=10];\n";
    os << "  rankdir=TB;\n";

    // nodes
    for (auto const& bb : g.blocks) {
        std::ostringstream label;
        label << "B" << bb.id << (bb.closed ? " (closed)\\l" : "\\l");
        for (auto const& ins : bb.instrs) {
            label << escape(ins) << "\\l";
        }
        os << "  B" << bb.id << " [label=\"" << label.str() << "\"];\n";
    }

    // paiting entry/exit
    os << "  B" << g.entry << " [style=filled, fillcolor=\"#cfe8ff\"];\n";
    os << "  B" << g.exit  << " [style=filled, fillcolor=\"#ffe6cf\"];\n";

    // edges
    for (auto const& bb : g.blocks) 
    {
        for (auto const& e : bb.outs) 
        {
            os << "  B" << bb.id << " -> B" << e.to;
            if (!e.label.empty()) os << " [label=\"" << escape(e.label) << "\"]";
            os << ";\n";
        }
    }

    os << "}\n";
}

void DotPrinter::Print(std::filesystem::path path, const cfg::Graph& g)
{
    std::ofstream os(path);
    std::cout << "DotPrinter: path=" << path << std::endl;
    if (!os.is_open()) { std::cout << "unable to open file!" << std::endl; return;}
    Print(os, g);
}


} // namespace pma::printers
