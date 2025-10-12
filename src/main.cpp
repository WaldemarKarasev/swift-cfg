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
    #if 1
    pma::App app;
    // if_else  
    // CFGTestFunc(app, "code_examples/if_else.swift", "if_else.json", "if_else.dot");
    // CFGTestFunc(app, "code_examples/for.swift", "for.json", "for.dot");
    // CFGTestFunc(app, "code_examples/for_break_continue.swift", "for_break_continue.json", "for_break_continue.dot");
    // CFGTestFunc(app, "code_examples/while.swift", "while.json", "while.dot");
    // CFGTestFunc(app, "code_examples/repeat_while.swift", "repeat_while.json", "repeat_while.dot");
    // CFGTestFunc(app, "code_examples/switch.swift", "switch.json", "switch.dot");
    // CFGTestFunc(app, "code_examples/func.swift", "func.json", "fun.dot");
    // CFGTestFunc(app, "code_examples/fib.swift", "fib.json", "fib.dot");
    // CFGTestFunc(app, "code_examples/func_2.swift", "func_2.json", "func_2.dot");
    CFGTestFunc(app, "code_examples/labled_switch.swift", "labled_switch.json", "labled_switch.dot");
    return 0;
    // return app.BuildCfg(argc, argv);
    
    #else   
    
    // Testing 
    pma::App app;
    
    // fallthrough
    CFGTestFunc(app, "code_examples/fallthrough.swift", "fallthrough.json", "fallthrough.dot");
    
    // for_break_continue
    CFGTestFunc(app, "code_examples/for_break_continue.swift", "for_break_continue.json", "for_break_continue.dot");
    
    // for
    CFGTestFunc(app, "code_examples/for.swift", "for.json", "for.dot");
    
    // func
    CFGTestFunc(app, "code_examples/func.swift", "func.json", "func.dot");
    
    // if_else
    CFGTestFunc(app, "code_examples/if_else.swift", "if_else.json", "if_else.dot");
    
    // nested_blocks
    CFGTestFunc(app, "code_examples/nested_blocks.swift", "nested_blocks.json", "nested_blocks.dot");
    
    // repeate_while
    CFGTestFunc(app, "code_examples/repeat_while.swift", "repeat_while.json", "repeat_while.dot");
    
    // switch
    CFGTestFunc(app, "code_examples/switch.swift", "switch.json", "switch.dot");
    
    // while
    CFGTestFunc(app, "code_examples/while.swift", "while.json", "while.dot");
    
    return 0;
    #endif
}

