#pragma once

#include <expected>
#include <span>
#include <string>

namespace PackageMan::Archive
{
std::expected<std::string, std::string> unpackTarGZFromMem(std::span<const std::byte> mem,
                                                           std::string_view fileName);
}