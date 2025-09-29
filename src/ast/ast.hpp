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
        Expr
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

    std::string init, cond, step;
    utils::SourceRange initR{}, condR{}, stepR{};
    std::unique_ptr<BlockStmt> body;
};

struct DoWhileStmt : Stmt 
{
    DoWhileStmt(): Stmt(DoWhile) {}

    utils::SourceRange condR{};
    std::string cond;
    std::unique_ptr<BlockStmt> body;
};

struct ReturnStmt : Stmt 
{
    ReturnStmt(): Stmt(Return) {}
    std::string expr;
    utils::SourceRange exprR{};
};

struct BreakStmt : Stmt 
{ 
    BreakStmt() : Stmt(Break) {} 
};

struct ContinueStmt : Stmt
{
    ContinueStmt() : Stmt(Continue) {} 
};

struct ExprStmt : Stmt 
{
    ExprStmt(): Stmt(Expr) {}
    std::string expr;
    utils::SourceRange exprR{};
};
    
} // namespace pma::ast
