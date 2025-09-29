// std
#include <iostream>
#include <string>

// pma
#include <frontend/registry.hpp>
#include <utils/source_range.hpp>
#include <diagnostic/diagnostic.hpp>
#include <ast/ast.hpp>
#include <printers/dot_printer.hpp>
int main() {
    // Swift code
    const char *source_code = R"(
        func test(x: Int) -> Int {
            if x > 0 {
                return x
            } else {
                return -x
            
        }
    )";

    std::string source(source_code);

    pma::utils::SourceView source_view(source);
    pma::diagnostic::StdOutDiagnostic diag(source_view);

    using Lang = pma::frontends::registry::Lang;
    using Frontend = pma::frontends::registry::Frontend;
    pma::frontends::registry::Registry::FrontendHandle swift_front = pma::frontends::registry::Registry::CreateFrontend(Lang::Swift, Frontend::TreeSitter);

    if (swift_front == nullptr) 
    {
        std::cout << "Swift-TreeSitter fronted wasn't found" << std::endl;
        return -1;
    }

    std::unique_ptr<pma::ast::BlockStmt> ast = swift_front->BuildFromRoot(source_view, diag);

    // build cfg

    // pma::printers::DotPrinter dot_printer;
    // dot_printer.Print("cfg.dot", )


}

