#pragma once

// std
#include <functional>
#include <map>

// pma
#include <frontend/ast_builder.hpp>

namespace pma::frontends::registry
{

enum class Lang : int
{ 
    Swift,
    // CPP,
    // Java,
    // ...
};

enum class Frontend : int
{
    TreeSitter,
    // Clang,
    // ...
};

class Registry
{
public:
    // using FactoryFn = std::function<std::unique_ptr<pma::frontends::IAstBuilder>()>;
    using FactoryFn = std::function<std::unique_ptr<IAstBuilder>()>;
    using FrontendHandle = std::unique_ptr<IAstBuilder>;
public:
    static void RegisterFrontend(Lang lang, Frontend front, FactoryFn factory);
    static FrontendHandle CreateFrontend(Lang lang, Frontend front);

private:
    static std::map<std::pair<Lang, Frontend>, FactoryFn> reg_;
};

struct AutoRegister
{
    AutoRegister(Lang lang, Frontend front, Registry::FactoryFn factory)
    {
        Registry::RegisterFrontend(lang, front, std::move(factory));
    }
};


} // namespace pma::frontends::registry
