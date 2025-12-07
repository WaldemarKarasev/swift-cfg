#include "metric_app.hpp"

#include <iostream>
#include <fstream>

#include <frontend/registry.hpp>
#include <utils/source_range.hpp>
#include <diagnostic/diagnostic.hpp>
#include <ast/ast.hpp>
#include <printers/dot_printer.hpp>
#include <printers/json_tree_serializer.hpp>

namespace pma
{
    
int MetricApp::Count(int argc, char** argv)
{
    std::string filename;
    std::string ast_filename;
    std::string metric_filename;
    
    #if 0
    std::cout << "argc=" << argc << std::endl;
    for (int i = 0; i < argc; ++i)
    {
        std::cout << "argc_" << i << ": " << argv[i] << std::endl;
    }
    #endif

    if (argc > 3)
    {
        filename = argv[1];
        metric_filename = argv[2];
        ast_filename = argv[3];
    }
    else if (argc > 2)
    {
        filename = argv[1];
        metric_filename = argv[2];
    }
    else if (argc > 1)
    {   
        filename = argv[1];
    }
    
    if (filename.empty())
    {
        std::cout << "filename is empty!" << std::endl;
        return -1;
    }

    if (metric_filename.empty())
    {
        metric_filename = "metric.json";
    }
            
    if (ast_filename.empty())
    {
        ast_filename = "ast.json";
    }

    return Count(filename, ast_filename, metric_filename);

}

int MetricApp::Count(std::string source_filename, std::string ast_filename, std::string metric_filename)
{
    std::cout << "filename: " << source_filename << std::endl;
    std::cout << "ast_filename: " << ast_filename << std::endl;
    std::cout << "dot_filename: " << metric_filename << std::endl;

    std::string source;
    std::ifstream file(source_filename);
    if (file.is_open())
    {
        std::string str;
        while (std::getline(file, str))
        {
            source += str + "\n";
        }
    }

    std::cout << "CODE FILENAME: " << source_filename << std::endl;
    if (source.empty())
    {
        std::cout << "failed to read code from file:" << source_filename << std::endl;
        return -2;
    }

    // std::cout << "[CODE START]:\n" << source << "\n [CODE END]" << std::endl;
    // std::string source(source_code);

    utils::SourceView source_view(source);
    diagnostic::StdOutDiagnostic diag(source_view);

    using Lang = frontends::Lang;
    using Frontend = frontends::Frontend;
    frontends::Registry::FrontendHandle swift_front = frontends::Registry::CreateFrontend(Lang::Swift, Frontend::TreeSitter);

    if (swift_front == nullptr) 
    {
        std::cout << "Swift-TreeSitter fronted wasn't found" << std::endl;
        return -3;
    }

    std::unique_ptr<ast::BlockStmt> ast = swift_front->BuildFromRoot(source_view, diag);

    printers::JsonTreeSerializer serializer;
    // serializer.Print(std::cout, *ast);
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

    // Coun metrics
    
    return 0;
}

} // namespace pma
