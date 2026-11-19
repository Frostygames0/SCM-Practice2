#pragma once

#include <expected>
#include <span>
#include <string>

namespace PackageMan::Archive
{
std::expected<std::string, std::string> readFromMemory(std::span<const std::byte> mem);
}