#include "repository.hpp"

#include <algorithm>
#include <format>
#include <fstream>
#include <iostream>

#include "archive.hpp"
#include "filedownload.hpp"
#include "utils/string.hpp"

namespace PackageMan
{
std::optional<std::vector<std::string>> ApkRepository::getDependencies(std::string_view name) const
{
    const auto* pkg = m_index.resolve(name);
    if (pkg == nullptr)
    {
        return std::nullopt;
    }

    std::vector<std::string> out;
    out.reserve(pkg->depends.size());
    for (const auto& dep : pkg->depends)
    {
        const auto* target = m_index.resolve(dep);
        out.push_back(target != nullptr ? target->name : dep);
    }

    // several so:* entries often resolve to the same package
    std::ranges::sort(out);
    auto [first, last] = std::ranges::unique(out);
    out.erase(first, last);
    return out;
}

std::expected<TestRepository, std::string> TestRepository::loadFromFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
    {
        return std::unexpected(std::format("cannot open test repo: {}", path));
    }

    TestRepository repo;
    std::string line;
    std::size_t lineNo = 0;
    while (std::getline(file, line))
    {
        ++lineNo;
        auto trimmed = StringUtils::trim(line);
        if (trimmed.empty() || trimmed.front() == '#')
        {
            continue;
        }

        auto colon = trimmed.find(':');
        if (colon == std::string_view::npos)
        {
            return std::unexpected(std::format("{}:{}: expected 'NAME: DEPS'", path, lineNo));
        }

        auto name = StringUtils::trim<std::string>(trimmed.substr(0, colon));
        auto deps = StringUtils::split<std::string>(trimmed.substr(colon + 1), ' ');
        repo.m_graph[std::move(name)] = std::move(deps);
    }

    return repo;
}

std::optional<std::vector<std::string>> TestRepository::getDependencies(std::string_view name) const
{
    if (auto it = m_graph.find(std::string{name}); it != m_graph.end())
    {
        return it->second;
    }
    return std::nullopt;
}

std::expected<std::unique_ptr<Repository>, std::string> makeRepository(const Config& cfg)
{
    if (cfg.testMode)
    {
        auto repo = TestRepository::loadFromFile(cfg.repoPath);
        if (!repo)
        {
            return std::unexpected(repo.error());
        }
        return std::make_unique<TestRepository>(*std::move(repo));
    }

    auto bytes = Web::downloadFrom(cfg.repoPath, [](std::uint64_t now, std::uint64_t total) {
        if (total > 0)
        {
            std::cerr << std::format(
                "\rDownloading: {:3}% ({}/{} KiB)", now * 100 / total, now / 1024, total / 1024);
        }
        else
        {
            std::cerr << std::format("\rDownloading: {} KiB", now / 1024);
        }
    });
    std::cerr << '\n';
    if (!bytes)
    {
        return std::unexpected(bytes.error());
    }

    auto text = Archive::unpackFileFromMem(*bytes, "APKINDEX");
    if (!text)
    {
        return std::unexpected(text.error());
    }

    return std::make_unique<ApkRepository>(ApkIndex::parseFromString(*text));
}
} // namespace PackageMan
