#pragma once

#include <expected>
#include <vector>
#include <string>

namespace PackageMan::Web
{
std::expected<std::vector<std::byte>, std::string> downloadFrom(const std::string& link);
}