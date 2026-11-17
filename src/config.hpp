#pragma once

#include <expected>
#include <ostream>
#include <string>

namespace PackageMan
{

struct Config
{
    std::string packageName;
    std::string repoPath;
    bool testMode = false;
    std::string outputImgName = "output.png";

    friend std::ostream& operator<<(std::ostream& out, const Config& cfg);
};

std::expected<void, std::string> validateConfig(const Config& cfg);

}; // namespace PackageMan