#pragma once

// std
#include <cstdint>
#include <utility>
#include <string>

namespace pma::utils
{

struct SourceRange
{
    uint32_t start_byte{};
    uint32_t end_byte{};
};

class SourceView 
{
public:
    explicit SourceView(const std::string& buffer) 
    : buffer_{buffer}
    {

    }

    std::string_view slice(const SourceRange& r) const 
    {
        const auto n = static_cast<uint32_t>(buffer_.size());
        const uint32_t s = std::min(r.start_byte, n);
        const uint32_t e = std::min(r.end_byte, n);
        if (e <= s) return {};
        return std::string_view(buffer_.data() + s, e - s);
    }

    const std::string& GetBuffer() const { return buffer_; }
    
private:
    const std::string& buffer_;
};

} // namespace cfg_builder::utils
