#include <cfg/cfg.hpp>
#include <cfg/cfg_builder.hpp>

// std
#include <iostream>
#include <optional>
#include <sstream>

using namespace pma;

namespace pma::cfg {

cfg::Graph CFGBuilder::Build(const ast::Stmt& root) 
{
    // std::cout << "CFGBuilder::Build start" << std::endl;
    g_ = {};
    
    g_.exit = g_.NewBlock();
    g_.entry = g_.NewBlock();

    func_exit_ = g_.exit;

    BuildOut top = build_stmt(root, {g_.entry});

    concat(top.outs, g_.exit);
    // std::cout << "CFGBuilder::Build end" << std::endl;
    return g_;
}

int CFGBuilder::new_block() 
{
    return g_.NewBlock();
}

void CFGBuilder::add_instr(int b, std::string s) 
{
    g_.AddInstr(b, std::move(s));
}

void CFGBuilder::edge(int from, int to, std::string lbl) 
{
    g_.AddEdge(from, to, std::move(lbl));
}

void CFGBuilder::seal(int b) 
{
    g_.CloseBlock(b);
}

void CFGBuilder::concat(const std::vector<int>& prev_outs, int next_entry) 
{
    for (int p : prev_outs) 
    {
        if (p == next_entry) continue;
        edge(p, next_entry);
    }
}

int CFGBuilder::ensure_single_open(std::vector<int>& in_outs)
{
    auto block_consolidation = [&](){
        int b = g_.NewBlock();
        if (!in_outs.empty()) concat(in_outs, b);
        in_outs.clear();
        in_outs.push_back(b);
        return b;
    };

    if (in_outs.empty()) return block_consolidation();
    if (in_outs.size() > 1) return block_consolidation();

    int b = in_outs.front();
    if (g_.IsBlockClosed(b)) return block_consolidation();
    return b;
}

BuildOut CFGBuilder::build_stmt(const ast::Stmt& s, std::vector<int> in_outs) 
{
    using K = ast::Stmt::Kind;
    switch (s.kind_) {
        case K::Expr:       return build_expr(static_cast<const ast::ExprStmt&>(s), in_outs);
        case K::Return:     return build_return(static_cast<const ast::ReturnStmt&>(s), in_outs);
        case K::Break:      return build_break(static_cast<const ast::BreakStmt&>(s), in_outs);
        case K::Continue:   return build_continue(static_cast<const ast::ContinueStmt&>(s), in_outs);
        case K::If:         return build_if(static_cast<const ast::IfStmt&>(s));
        case K::While:      return build_while(static_cast<const ast::WhileStmt&>(s), in_outs);
        case K::DoWhile:    return build_do_while(static_cast<const ast::DoWhileStmt&>(s), in_outs);
        case K::For:        return build_for_each(static_cast<const ast::ForStmt&>(s), in_outs);
        case K::Switch:     return build_switch(static_cast<const ast::SwitchStmt&>(s), in_outs);
        case K::Block:      return build_block(static_cast<const ast::BlockStmt&>(s), in_outs);
        case K::Lable:      return build_lable(static_cast<const ast::LableStmt&>(s), in_outs);
        case K::Fallthrough: return build_fallthrough(static_cast<const ast::FallthroughStmt&>(s), in_outs);
        case K::Function:    return build_func(static_cast<const ast::FunctionDeclStmt&>(s), in_outs);
        default:            
            int b = ensure_single_open(in_outs);  add_instr(b, "/* unsupported stmt */");
            return { b, { b } };
    }
}

BuildOut CFGBuilder::build_seq(const std::vector<std::unique_ptr<ast::Stmt>>& stmts, std::vector<int> in_outs) 
{
    // Entry for builded sequence will be taken as entry of the first child(one of the expr in stmts.body)
    int entry = -1; // is sequence entry initialized
    std::vector<int> outs = std::move(in_outs); // dangling outputs from the top basic block/blocks

    for (auto& uptr : stmts)
    {
        // Pass top basic block outputs as input outs to the child
        BuildOut child_hubs = build_stmt(*uptr, outs);

        // if child has an entry
        if (child_hubs.entry != -1) 
        {
            // if outputs of current sequence aren't empty, then connect current ouputs to the child entry
            if (!outs.empty()) concat(outs, child_hubs.entry);
            // if entry block isn't initialized then set entry for current sequence to the child entry,
            // so our first child with input will be our first basic block in the builded sequence of blocks 
            if (entry == -1) entry = child_hubs.entry;
        }
        // Updating outputs for current building sequence/block for outputs of just builded child
        outs = std::move(child_hubs.outs);
    }

    // if entry wasn't initialized from any children, then we will open an empty block and pass this block as a entry for our builded sequence
    if (entry == -1) 
    {
        int b = ensure_single_open(outs);
        entry = b;
    }

    return { entry, std::move(outs) };
}

BuildOut CFGBuilder::build_block(const ast::BlockStmt& s, std::vector<int> in_outs) 
{
    // std::cout << "CFGBuilder::build_block start" << std::endl;
    return build_seq(s.stmts_, in_outs);
    // std::cout << "CFGBuilder::build_block end" << std::endl;
}

BuildOut CFGBuilder::build_expr(const ast::ExprStmt& s, std::vector<int> in_outs) 
{
    int b = ensure_single_open(in_outs);
    // std::cout << "CFGBuilder::build_expr cur_=" << b << "; expr=" << s.expr << std::endl;
    add_instr(b, "expr: " + s.expr);
    // std::cout << "CFGBuilder::build_expr end" << std::endl;
    return { b, { b } };
}

BuildOut CFGBuilder::build_if(const ast::IfStmt& s) 
{
    // Always create a NEW basic block and don't use the input_outputs
    // Parent who called build_if connect by itself connect its outs into our cond basic block
    
    // condition basic block
    int cond = new_block();
    add_instr(cond, s.cond);  

    // then/else basic blocks are building without inputs.
    BuildOut thenB = build_stmt(*s.thenB, {});
    std::vector<int> outs; // outputs for if_stmt block

    // if_stmt has a else branch 
    if (s.elseB) 
    {
        // building else branch
        BuildOut elseB = build_stmt(*s.elseB, {});
        edge(cond, thenB.entry, "true"); // connecing true branch to thenBrach
        edge(cond, elseB.entry, "false"); // connecing false branch to elseBrach
        outs = thenB.outs; // outputs += outputs from thenBranch
        outs.insert(outs.end(), elseB.outs.begin(), elseB.outs.end()); // if_stmt outputs is current outputs + elseBranch outputs
    } 
    else // if_stmts hasn't else branch 
    {
        int join = new_block(); // join block to connect elseBranch into one output
        edge(cond, thenB.entry, "true"); // connecing true branch to thenBrach
        edge(cond, join, "false"); // connecing false branch to elseBrach
        concat(thenB.outs, join); // connecting all thenBranch outputs into join block
        outs = { join }; // outputs = join block = all outputs from thenBranch
    }

    return { cond, std::move(outs) };
}

BuildOut CFGBuilder::build_for_each(const ast::ForStmt& s, std::vector<int> in_outs)
{
    // consume pending label
    std::string loop_label = pending_lable_.has_value() ? *pending_lable_ : "";
    pending_lable_.reset();

    const int after = new_block(); // after block for the end of cycle
    const int init  = new_block(); // initialization cycle block 
    const int next  = new_block(); // block for performing retrieving new iterator for the next loop iteration

    add_instr(init, "lable " + loop_label + ":"); // adding lable to the loop
    add_instr(init, "seq = " + s.collection); // sequence initializaion instruction
    add_instr(init, s.item + " = make_iterator(seq)"); // creating iterator instruction
    edge(init, next, loop_label.empty() ? "for-in" : "for-in " + loop_label); // connecting init block with next block with loop label

    add_instr(next, s.item + " = seq.next()"); // retrieving next iterator 
    add_instr(next, "if " + s.item + " == nil "); // condition for continuing cycle
    add_instr(next, loop_label.empty() ? "for-next" : "for-next " + loop_label); // adding loop label to the next block

    // if label isn't empty then mark after block with this lable
    if (!loop_label.empty())
    {
        add_instr(after, "for-break " + loop_label);
    }

    // std::cout << "loop_lable=" << loop_label << std::endl;
    // updating cycle stack
    loop_stack_.push_back({ after, next, loop_label });

    // building body of cycle
    BuildOut body = build_stmt(*s.body, {});
    edge(next, body.entry, "true"); // connection for continuing cycle
    edge(next, after,      "false"); // connection for stopping the cycle
    concat(body.outs, next); // connecting all cycle body outputs into next block

    loop_stack_.pop_back(); // deleting current cycle context from the cycle context
    return { init, { after } };
}

BuildOut CFGBuilder::build_lable(const ast::LableStmt& lable, std::vector<int> in_outs)
{
    pending_lable_ = lable.lable;    
    // std::cout << "lable.lable=" << lable.lable << std::endl;
    return {-1, std::move(in_outs)};
}

std::pair<int,int> CFGBuilder::resolve_loop_targets(const std::optional<std::string>& label) 
{
    if (!label || label->empty()) 
    {
        if (loop_stack_.empty()) return { -1, -1 };
        const auto& top = loop_stack_.back();
        return { top.break_tgt, top.cont_tgt };
    }
    const std::string want = *label;
    for (auto it = loop_stack_.rbegin(); it != loop_stack_.rend(); ++it) 
    {
        if (it->label == want) return { it->break_tgt, it->cont_tgt };
    }

    for (auto it = switch_stack_.rbegin(); it != switch_stack_.rend(); ++it) 
    {
        if (it->label == want) return { it->break_tgt, -1 };
    }

    return { -1, -1 }; // fallback, should be unreacheable
}

BuildOut CFGBuilder::build_break(const ast::BreakStmt& s, std::vector<int> in_outs) 
{
    int b = ensure_single_open(in_outs);
    auto [br, ct] = resolve_loop_targets(s.lable); // s.label: std::optional<std::string>
    add_instr(b, s.has_lable ? ("break " + s.lable) : "break");
    edge(b, br, "break");
    // seal(b);
    return { b, {} };
}

BuildOut CFGBuilder::build_continue(const ast::ContinueStmt& s, std::vector<int> in_outs) {
    int b = ensure_single_open(in_outs);
    auto [br, ct] = resolve_loop_targets(s.lable);
    add_instr(b, s.has_lable ? ("continue " + s.lable) : "continue");
    edge(b, ct, "continue");
    // seal(b);
    return { b, {} };
}

BuildOut CFGBuilder::build_while(const ast::WhileStmt& s, std::vector<int> in_outs)
{
    // cycle blocks
    const int after = new_block();   // cycle outputs
    const int cond  = new_block();   // cycle condition block

    // loop lable
    std::string loop_label = pending_lable_.value_or("");
    pending_lable_.reset();

    // printing labels, conditions in blocks
    if (!loop_label.empty()) add_instr(cond,  "label " + loop_label + ":");
    add_instr(cond, s.cond); 
    add_instr(cond, loop_label.empty() ? "while-cond" : "while-cond [" + loop_label + "]");
    add_instr(after, loop_label.empty() ? "while-after" : "while-after [" + loop_label + "]");

    // cycle context break/continue: continue -> cond, break -> after
    loop_stack_.push_back({ after, cond, loop_label });

    // building cycle body
    BuildOut body = build_stmt(*s.body, {});

    // connecting: cond.true -> body, cond.false -> after
    edge(cond, body.entry, "true");
    edge(cond, after,      "false");

    // connecting outputs from body to condition block
    concat(body.outs, cond);

    loop_stack_.pop_back();

    // Entry - cond; output - after
    return { cond, { after } };
}

BuildOut CFGBuilder::build_do_while(const ast::DoWhileStmt& s, std::vector<int> in_outs)
{
    // cycles blocks
    const int after = new_block();   // end cycle block
    const int cond  = new_block();   // condition block

    // loop label
    std::string loop_label = pending_lable_.value_or("");
    pending_lable_.reset();

    // Printing labels, condition into their blocks
    if (!loop_label.empty()) add_instr(cond, "label " + loop_label + ":");
    add_instr(cond, s.cond); 
    add_instr(cond, loop_label.empty() ? "repeat-while-cond" : "repeat-while-cond [" + loop_label + "]");
    add_instr(after, loop_label.empty() ? "repeat-after" : "repeat-after [" + loop_label + "]");

    // loop context: continue -> cond, break -> after
    loop_stack_.push_back({ after, cond, loop_label });

    // building cycle body
    BuildOut body = build_stmt(*s.body, {});

    // connecting body's outputs into condition block
    concat(body.outs, cond);

    // connecting cond: true -> body.entry; false -> after
    edge(cond, body.entry, "true");
    edge(cond, after,      "false");

    loop_stack_.pop_back();

    // Entry — cycle body, output - after
    return { body.entry, { after } };
}

BuildOut CFGBuilder::build_switch(const ast::SwitchStmt& s, std::vector<int> in_outs)
{
    // Switch blocks
    const int after    = new_block();     // output
    const int dispatch = new_block();     // entry

    // switch lable
    std::string sw_label = pending_lable_.value_or("");
    pending_lable_.reset();

    if (!sw_label.empty()) add_instr(dispatch, "label " + sw_label + ":");
    // switch condition
    add_instr(dispatch, "switch " + s.condition);

    // Pushin switch context onto the switch_stack_ for supporting labeled/unlabeled break
    switch_stack_.push_back({ after, std::nullopt, sw_label });

    // Assume that an ast::SwitchStmt has:
    //  s.cases : std::vector<std::unique_ptr<ast::SwitchCaseStmt>>
    //  each s.case[i] has a body
    //  s.cases[last] == default case
    
    const size_t N = s.cases.size(); // number of cases stmts
    std::vector<int> caseChk(N, -1); // case chuncks
    std::vector<BuildOut> bodies(N); // case bodies

    // Creating caseChunks: body, conditions and etc. 
    for (size_t i = 0; i < N; ++i) 
    {
        caseChk[i] = new_block();

        std::string header = s.cases[i]->is_default ? "default" : ("case " + s.cases[i]->pattern);
        if (s.cases[i]->guard != nullptr)
        {
            header += " where " + s.cases[i]->guard->expr;
        }
        // std::cout << "header=" << header << std::endl;
        add_instr(caseChk[i], header); // adding condition header into caseChunk basic block

        // building case's body
        bodies[i] = build_stmt(*s.cases[i]->body, {});
    }

    // Creating dispatch chain
    if (N > 0) edge(dispatch, caseChk[0], "dispatch");

    for (size_t i = 0; i < N; ++i) 
    {
        // connecting match case into the case's body
        edge(caseChk[i], bodies[i].entry, "match");

        // connect no-match edge into the next caseChunk. Assume that default case will be last
        if (i + 1 < N) 
        {
            edge(caseChk[i], caseChk[i + 1], "no-match");
        }
        else 
        {
            // defaul case doens't have no-match edge
            // edge(caseChk[i], after, "no-match");
        }
    }

    // Case's terminators handling
    for (size_t i = 0; i < N; ++i) {
        auto term = s.cases[i]->terminator; // None / Break / Fallthrough / Return

        if (term == ast::SwitchCaseStmt::Terminator::Fallthrough) 
        {
            if (i + 1 < N) 
            {
                // fallthrough into the nect block
                concat(bodies[i].outs, caseChk[i + 1]);
            } else
            {
                // connecting case's body into the after block
                concat(bodies[i].outs, after);
            }
        }
        else 
        {
            concat(bodies[i].outs, after);
        }
    }

    switch_stack_.pop_back();

    // Entry block - dispatch and one output - after
    return { dispatch, { after } };

}

BuildOut CFGBuilder::build_func(const ast::FunctionDeclStmt& s, std::vector<int> in_outs)
{
    int prologe = new_block(); // creating block with signature description in it

    // signature string retrieving from ast::FunctionDeclStmt
    std::stringstream sig;
    sig << "func " << s.signature.name << " (";

    for (size_t i = 0; i < s.signature.params.size(); ++i)
    {
        const auto& param = s.signature.params[i];
        sig << (param.external_name.empty() ? "_" : param.external_name) 
        << " " 
        << param.local_name 
        << " : " << param.type_name;
        if (i + 1 != s.signature.params.size()) sig << ", ";
    }

    sig << ")";

    if (!s.signature.return_type.empty())
    {
        sig << " -> " << s.signature.return_type;
    }
    else
    {
        sig << " -> Void";
    }
    
    // adding function signature string into prologe block
    add_instr(prologe, sig.str()); 
    
    // building functions body
    BuildOut body = build_block(*s.body, {}); 

    // connecting prologe block to function body input
    edge(prologe, body.entry);

    // returning prologe block as an input  and a function's body outputs as an outputs of the funciton declaration stmt 
    return {prologe, body.outs};
}

BuildOut CFGBuilder::build_return(const ast::ReturnStmt& s, std::vector<int> in_outs)
{
    int b = ensure_single_open(in_outs);
    add_instr(b, "return " + s.lable);
    if (func_exit_ >= 0) edge(b, func_exit_, "return");
    seal(b);
    return { b, {} };
}

BuildOut CFGBuilder::build_fallthrough(const ast::FallthroughStmt& s, std::vector<int> in_outs)
{
    int b = ensure_single_open(in_outs);
    std::cout << "CFGBuilder::build_fallthrough cur_=" << b << "; expr=" << s.lable << std::endl;
    add_instr(b, "fallthrough");
    // std::cout << "CFGBuilder::build_expr end" << std::endl;
    return { b, { b } };
}

} // namespace pma::cfgbuild
