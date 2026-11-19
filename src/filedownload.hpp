#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <string>
#include <vector>

namespace PackageMan::Web
{
// (downloaded bytes, total bytes; 0 if unknown)
using ProgressCallback = std::function<void(std::uint64_t, std::uint64_t)>;

std::expected<std::vector<std::byte>, std::string> downloadFrom(
    const std::string& link, const ProgressCallback& onProgress = {});
} // namespace PackageMan::Web
