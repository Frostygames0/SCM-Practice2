#pragma once

#include <expected>
#include <string>

namespace PackageMan
{
std::expected<void, std::string> renderMermaid(const std::string& mermaid,
                                               const std::string& outputPath);
} // namespace PackageMan
