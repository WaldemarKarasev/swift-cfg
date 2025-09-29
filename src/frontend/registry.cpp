#include <frontend/registry.hpp>

namespace pma::frontends::registry
{         

std::map<std::pair<Lang, Frontend>, Registry::FactoryFn> Registry::reg_ = {}; // zero init 

void Registry::RegisterFrontend(Lang lang, Frontend front, FactoryFn factory)
{
    reg_[{lang, front}] = factory;
    return;
}

Registry::FrontendHandle Registry::CreateFrontend(Lang lang, Frontend front)
{
    auto it = reg_.find({lang, front});
    return it == reg_.end() ? nullptr : it->second();
}

} // namespace pma::frontends::registry
