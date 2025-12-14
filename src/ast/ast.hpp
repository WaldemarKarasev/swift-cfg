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
        Function,
        Class,
        VarDecl,
        Identifier,
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

struct ExprBase
{
    enum Kind
    {
        Unknown,
        Identifier,
        MemberAccess,
        Call,
        // TODO: Literal, Binary, ...
    };

    Kind kind;
    utils::SourceRange range{};
    std::string text;
    
    ExprBase(Kind k) : kind(k) {}
    virtual ~ExprBase() = default;
};

using ExprPtr = std::unique_ptr<ExprBase>;

struct ExprStmt : Stmt
{
    std::string text;
    std::unique_ptr<ExprBase> expr;

    ExprStmt()
        : Stmt(Stmt::Expr) {}

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

// ------------------- Funtion declaration ----------------------
struct Param
{
    std::string external_name{};
    std::string local_name{};
    std::string type_name{};

    utils::SourceRange external_nameR{};
    utils::SourceRange local_nameR{};
    utils::SourceRange type_nameR{};
};

struct FunctionSignature
{
    std::string name;
    utils::SourceRange nameR;
    std::vector<Param> params;
    std::string return_type; // "" -> Void
    utils::SourceRange return_typeR{};
};

struct FunctionDeclStmt : Stmt
{
    FunctionSignature signature;
    std::unique_ptr<BlockStmt> body;
    
    // meta data
    bool is_initializer = false;  // init
    bool is_deinitializer = false; // deinit
    bool is_static = false;      // static func / class func
    bool is_override = false;    // override func

    FunctionDeclStmt() : Stmt(Stmt::Function) {}
};

struct ClassDeclStmt : Stmt 
{
    // class name
    std::string name;                     
    utils::SourceRange nameR{};

    // base class "class Foo: Bar" Bar - baseclass) or "" - empty
    std::string base_class_name;          
    utils::SourceRange base_class_nameR{};

    // class members, functions, ...
    std::vector<std::unique_ptr<Stmt>> members;

    ClassDeclStmt() : Stmt(Stmt::Class) {}
};

// ----- Expresions -----

struct VarDeclStmt : Stmt 
{
    std::string name;
    utils::SourceRange nameR{};

    std::string type_name;
    utils::SourceRange type_nameR{};

    // initializer expr
    std::unique_ptr<ExprStmt> initializer;

    bool is_property = false; // true if it is class member

    VarDeclStmt() : Stmt(Stmt::VarDecl) {}
};

struct UnknownExpr : ExprBase
{
    UnknownExpr() : ExprBase(Unknown) {}
};

struct IdentifierExpr : ExprBase
{
    std::string name;
    utils::SourceRange nameR{};

    IdentifierExpr() : ExprBase(Identifier) {}
};

struct MemberAccessExpr : ExprBase 
{
    // Expr before "." (self, object, TypeName)
    std::unique_ptr<ExprBase> base;  
    std::string member_name;
    utils::SourceRange member_nameR{};

    MemberAccessExpr() : ExprBase(MemberAccess) {}
};

struct CallExpr : ExprBase 
{
    // IdentifierExpr, MemberAccessExpr, TypeName.init
    std::unique_ptr<ExprBase> callee;           
    std::vector<std::unique_ptr<ExprBase>> args;

    CallExpr() : ExprBase(Call) {}
};

} // namespace pma::ast
