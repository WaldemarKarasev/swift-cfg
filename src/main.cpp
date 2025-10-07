// std
#include <iostream>
#include <string>
#include <fstream>

// pma
#include <frontend/registry.hpp>
#include <utils/source_range.hpp>
#include <diagnostic/diagnostic.hpp>
#include <ast/ast.hpp>
#include <printers/dot_printer.hpp>
#include <printers/json_tree_serializer.hpp>

int main(int argc, char** argv) {

    std::string filename;
    std::string ast_filename;
    std::string dot_filename;
    std::cout << "argc=" << argc << std::endl;

    for (int i = 0; i < argc; ++i)
    {
        std::cout << "argc_" << i << ": " << argv[i] << std::endl;
    }

    if (argc > 3)
    {
        filename = argv[1];
        ast_filename = argv[2];
        dot_filename = argv[3];
    }
    else if (argc > 2)
    {
        filename = argv[1];
        ast_filename = argv[2];
    }
    else if (argc > 1)
    {   
        filename = argv[1];
    }
    else
    {
        filename = "code_examples/if_else.swift";
        ast_filename = "ast.json";
        dot_filename = "cfg.dot";
    }

    std::cout << "filename: " << filename << std::endl;
    std::cout << "ast_filename: " << ast_filename << std::endl;
    std::cout << "dot_filename: " << dot_filename << std::endl;

    std::string source;
    std::ifstream file(filename);
    if (file.is_open())
    {
        std::string str;
        while (std::getline(file, str))
        {
            source += str + "\n";
        }
    }

    std::cout << "CODE FILENAME: " << filename << std::endl;
    if (source.empty())
    {
        std::cout << "failed to read code from file:" << filename << std::endl;
        return -1;
    }

    std::cout << "[CODE START]:\n" << source << "\n [CODE END]" << std::endl;
    // std::string source(source_code);

    pma::utils::SourceView source_view(source);
    pma::diagnostic::StdOutDiagnostic diag(source_view);

    using Lang = pma::frontends::Lang;
    using Frontend = pma::frontends::Frontend;
    pma::frontends::Registry::FrontendHandle swift_front = pma::frontends::Registry::CreateFrontend(Lang::Swift, Frontend::TreeSitter);

    if (swift_front == nullptr) 
    {
        std::cout << "Swift-TreeSitter fronted wasn't found" << std::endl;
        return -1;
    }

    std::unique_ptr<pma::ast::BlockStmt> ast = swift_front->BuildFromRoot(source_view, diag);

    pma::printers::JsonTreeSerializer serializer;
    serializer.Print(std::cout, *ast);

    {
        std::ofstream ast_file(ast_filename);
        if (ast_file.is_open())
        {
            serializer.Print(ast_file, *ast);
        }
        else
        {
            std::cout << "Cannot open file: " << ast_filename << std::endl;
        }
    }

    // build cfg

    // pma::printers::DotPrinter dot_printer;
    // dot_printer.Print("cfg.dot", )


}

