#pragma once

#pragma once

// std
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// pma
#include <cfg/cfg.hpp>
#include <ast/ast.hpp>

namespace pma::cfg {

struct BuildOut 
{
    int entry{-1};
    std::vector<int> outs;
};

class CFGBuilder 
{
public:
    cfg::Graph Build(const ast::Stmt& root); // root = Block / Script / FunctionBody

private:
    // current builded cfg graph and current build state
    cfg::Graph g_;
    int func_exit_{-1};

    struct LoopCtx 
    { 
        int break_tgt{-1};
        int cont_tgt{-1}; 
        std::string label; 
    };

    struct SwitchCtx 
    { 
        int break_tgt{-1}; 
        std::optional<int> fall_to_next; 
        std::string label; 
    };

    std::vector<LoopCtx>   loop_stack_;
    std::vector<SwitchCtx> switch_stack_;
    std::unordered_map<std::string, std::pair<int,int>> label_map_; // label -> {break,continue}
    std::optional<std::string> pending_lable_{};
private:
    int  new_block();
    void add_instr(int b, std::string s);
    void edge(int from, int to, std::string lbl="");
    void seal(int b);
    void concat(const std::vector<int>& prev_outs, int next_entry);
    int ensure_single_open(std::vector<int>& in_outs);


    BuildOut build_stmt(const ast::Stmt& s, std::vector<int> in_outs);
    BuildOut build_seq(const std::vector<std::unique_ptr<ast::Stmt>>& stmts, std::vector<int> in_outs);
    BuildOut build_block(const ast::BlockStmt& s, std::vector<int> in_outs);
    BuildOut build_expr(const ast::ExprStmt& s, std::vector<int> in_outs);
    BuildOut build_if(const ast::IfStmt& s);
    BuildOut build_for_each(const ast::ForStmt& s, std::vector<int> in_outs);
    BuildOut build_lable(const ast::LableStmt& lable, std::vector<int> in_outs);
    std::pair<int,int> resolve_loop_targets(const std::optional<std::string>& label);
    BuildOut build_break(const ast::BreakStmt& s, std::vector<int> in_outs);
    BuildOut build_continue(const ast::ContinueStmt& s, std::vector<int> in_outs);
    BuildOut build_while(const ast::WhileStmt& s, std::vector<int> in_outs);
    BuildOut build_do_while(const ast::DoWhileStmt& s, std::vector<int> in_outs);
    BuildOut build_switch(const ast::SwitchStmt& s, std::vector<int> in_outs);
    BuildOut build_func(const ast::FunctionDeclStmt& s, std::vector<int> in_outs);
    BuildOut build_return(const ast::ReturnStmt& s, std::vector<int> in_outs);
    BuildOut build_fallthrough(const ast::FallthroughStmt& s, std::vector<int> in_outs);
};



} // namespace pma::cfgbuild
