// std
#include <iostream>
#include <string>
#include <fstream>

// pma
#include <metric_app.hpp>

void MetricTestFunc(pma::MetricApp& app, std::string source, std::string out_ast, std::string out_json)
{
    std::cout << "TEST: " << source << std::endl;
    app.Count(source, out_ast, out_json);
}


int MetricTestAll()
{
    // Testing 
    pma::MetricApp app;
    
    // cbo_example
    MetricTestFunc(app, "oop_code_examples/cbo_example.swift", "ast_cbo_example.json", "cbo_example.json");

    // inheritance_example
    MetricTestFunc(app, "oop_code_examples/inheritance_example.swift", "ast_inheritance_example.json", "inheritance_example.json");

    // lcom_example
    MetricTestFunc(app, "oop_code_examples/lcom_example.swift", "ast_lcom_example.json", "lcom_example.json");
    
    // rfc_example
    MetricTestFunc(app, "oop_code_examples/rfc_example.swift", "ast_rfc_example.json", "rfc_example.json");

    // wmc_example
    MetricTestFunc(app, "oop_code_examples/wmc_example.swift", "ast_wmc_example.json", "wmc_example.json");
    
    return 0;
}

int main(int argc, char** argv) 
{
    #if 1

    for (int i = 0; i < argc; ++i)
    {
        std::cout << argv[i] << std::endl;
    }

    pma::MetricApp app;
    return app.Count(argc, argv);

    #elif 0
    pma::App app;
    // if_else  
    // MetricTestAll(app, "oop_code_examples/if_else.swift", "if_else.json", "if_else.dot");
    // MetricTestAll(app, "oop_code_examples/for.swift", "for.json", "for.dot");
    // MetricTestAll(app, "oop_code_examples/for_break_continue.swift", "for_break_continue.json", "for_break_continue.dot");
    // MetricTestAll(app, "oop_code_examples/while.swift", "while.json", "while.dot");
    // MetricTestAll(app, "oop_code_examples/repeat_while.swift", "repeat_while.json", "repeat_while.dot");
    // MetricTestAll(app, "oop_code_examples/switch.swift", "switch.json", "switch.dot");
    // MetricTestAll(app, "oop_code_examples/func.swift", "func.json", "fun.dot");
    // MetricTestAll(app, "oop_code_examples/fib.swift", "fib.json", "fib.dot");
    // MetricTestAll(app, "oop_code_examples/func_2.swift", "func_2.json", "func_2.dot");
    MetricTestAll(app, "oop_code_examples/labled_switch.swift", "labled_switch.json", "labled_switch.dot");
    return 0;
    // return app.BuildCfg(argc, argv);
    #endif
}

