#pragma once

// std
#include <vector>
#include <memory>

// pma
#include <utils/source_range.hpp>


namespace pma::ast
{

struct Stmt 
{
    enum Kind
    {
        Block,
        If, 
        While, 
        For, 
        DoWhile, 
        Return, 
        Break, 
        Continue, 
        Fallthrough,
        Expr,
        Switch,
        SwitchCase,
        Lable,
    };
        
    explicit Stmt(Kind k): kind_(k) {}
    virtual ~Stmt() = default;
    
    Kind kind_;
    utils::SourceRange range{};
};

using StmtPtr = std::unique_ptr<Stmt>;

struct BlockStmt : Stmt 
{
    BlockStmt(): Stmt(Block) {}

    std::vector<StmtPtr> stmts_;
};

struct IfStmt : Stmt 
{
    IfStmt(): Stmt(If) {}
    
    std::string cond;
    utils::SourceRange condR{};
    std::unique_ptr<BlockStmt> thenB, elseB; // else can be nullptr
};

struct WhileStmt : Stmt 
{
    WhileStmt(): Stmt(While) {}

    std::string cond;
    utils::SourceRange condR{};
    std::unique_ptr<BlockStmt> body;
};

struct ForStmt : Stmt 
{
    ForStmt(): Stmt(For) {}

    std::string item, collection;
    utils::SourceRange itemR{}, collectionR{};
    std::unique_ptr<BlockStmt> body;
};

struct DoWhileStmt : Stmt 
{
    DoWhileStmt(): Stmt(DoWhile) {}

    utils::SourceRange condR{};
    std::string cond;
    std::unique_ptr<BlockStmt> body;
};

struct ControlStmt : Stmt
{
    ControlStmt(Kind kind) : Stmt(kind) {}
    bool has_lable = false;
    std::string lable;
    utils::SourceRange lableR{};
};

struct ReturnStmt : ControlStmt
{
    ReturnStmt(): ControlStmt(Return) {}
};

struct BreakStmt : ControlStmt 
{ 
    BreakStmt() : ControlStmt(Break) {} 
};

struct ContinueStmt : ControlStmt
{
    ContinueStmt() : ControlStmt(Continue) {} 
};

struct FallthroughStmt : ControlStmt
{
    FallthroughStmt() : ControlStmt(Fallthrough) {}
    bool synthetic = false;
};

struct ExprStmt : Stmt 
{
    ExprStmt(): Stmt(Expr) {}
    std::string expr;
    utils::SourceRange exprR{};
};
    
// -------------------- Switch/Case --------------------

struct SwitchCaseStmt : Stmt
{
    enum class Terminator {
        None,
        Break,
        Fallthrough,
    };

    Terminator terminator = Terminator::None;
    bool is_default = false;
    std::string pattern;
    std::unique_ptr<ExprStmt> guard;
    std::unique_ptr<BlockStmt> body; 

    SwitchCaseStmt() : Stmt(Stmt::SwitchCase) {}
};

struct SwitchStmt : Stmt
{
    std::string condition;           
    utils::SourceRange condition_range;

    std::vector<std::unique_ptr<SwitchCaseStmt>> cases; 

    SwitchStmt() : Stmt(Stmt::Switch) {}
};

// ------------------- Lable ----------------------
struct LableStmt : Stmt
{
    std::string lable{};
    utils::SourceRange lableR{};

    LableStmt() : Stmt(Stmt::Lable) {}
};

} // namespace pma::ast
