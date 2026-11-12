#pragma once

#include <expected>
#include <ostream>
#include <string>
#include <string_view>

namespace PackageMan
{

inline constexpr std::string_view ImgExt = ".png";

struct Config
{
    std::string packageName;
    std::string repoPath;
    bool testMode;
    std::string outputImgName;

    friend std::ostream& operator<<(std::ostream& out, const Config& cfg);
};

std::expected<void, std::string> validateConfig(const Config& cfg);

}; // namespace PackageMan