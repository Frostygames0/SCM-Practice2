#include "config.hpp"

#include <algorithm>
#include <expected>
#include <filesystem>
#include <format>
#include <ostream>
#include <string>
#include <system_error>

#include "CLI11/CLI11.hpp"

namespace PackageMan
{
namespace
{
bool isUpperLatin(const std::string& s)
{
    return !s.empty() && std::ranges::all_of(s, [](char c) { return c >= 'A' && c <= 'Z'; });
}
} // namespace

std::ostream& operator<<(std::ostream& out, const Config& cfg)
{
    return out << "[Config]\n"
               << "package: " << cfg.packageName << '\n'
               << "repo: " << cfg.repoPath << '\n'
               << "test_mode: " << (cfg.testMode ? "true" : "false") << '\n'
               << "output: " << cfg.outputImgName << '\n';
}

std::expected<Config, int> parseFromCommandLine(int argc, char** argv)
{
    PackageMan::Config cfg;
    CLI::App app{"PackageMan: Alpine (apk) dependency graph visualizer"};
    app.add_option("-p,--package", cfg.packageName, "Package name")->required();
    app.add_option("-r,--repo", cfg.repoPath, "Repository URL or test file path")->required();
    app.add_flag("-t,--test", cfg.testMode, "Use test repository file");
    app.add_option("-o,--output", cfg.outputImgName, "Output PNG file name");

    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError& e)
    {
        return std::unexpected{app.exit(e)};
    }

    return cfg;
}

std::expected<void, std::string> validateConfig(const Config& cfg)
{
    if (cfg.packageName.empty())
    {
        return std::unexpected("Package Name must not be empty!");
    }

    if (cfg.testMode)
    {
        std::error_code ec;
        if (!std::filesystem::is_regular_file(cfg.repoPath, ec))
        {
            return std::unexpected(std::format("test repo file not found: {}", cfg.repoPath));
        }
        if (!isUpperLatin(cfg.packageName))
        {
            return std::unexpected("in test mode package name must be upper-case latin letters");
        }
    }
    else
    {
        if (!cfg.repoPath.starts_with("http://") && !cfg.repoPath.starts_with("https://"))
        {
            return std::unexpected("Repo must be a valid HTTP link!");
        }
    }

    return {};
}
} // namespace PackageMan
