// std
#include <iostream>
#include <string>
#include <fstream>

// pma
#include <app.hpp>

void CFGTestFunc(pma::App& app, std::string source, std::string out_ast, std::string out_dot)
{
    app.BuildCfg(source, out_ast, out_dot);
}

int main(int argc, char** argv) 
{
    #if 0
    pma::App app;
    // if_else
    // CFGTestFunc(app, "code_examples/func.swift", "func.json", "");
    CFGTestFunc(app, "code_examples/func.swift", "func.json", "");
    return 0;
    // return app.BuildCfg(argc, argv);
    
    #else   
    
    // Testing 
    pma::App app;
    
    // fallthrough
    CFGTestFunc(app, "code_examples/fallthrough.swift", "fallthrough.json", "");
    
    // for_break_continue
    CFGTestFunc(app, "code_examples/for_break_continue.swift", "for_break_continue.json", "");
    
    // for
    CFGTestFunc(app, "code_examples/for.swift", "for.json", "");
    
    // func
    CFGTestFunc(app, "code_examples/func.swift", "func.json", "");
    
    // if_else
    CFGTestFunc(app, "code_examples/if_else.swift", "if_else.json", "");
    
    // nested_blocks
    CFGTestFunc(app, "code_examples/nested_blocks.swift", "nested_blocks.json", "");
    
    // repeate_while
    CFGTestFunc(app, "code_examples/repeat_while.swift", "repeat_while.json", "");
    
    // switch
    CFGTestFunc(app, "code_examples/switch.swift", "switch.json", "");
    
    // while
    CFGTestFunc(app, "code_examples/while.swift", "while.json", "");
    
    return 0;
    #endif
}

