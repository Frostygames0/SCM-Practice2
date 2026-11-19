#pragma once

#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "apkindex.hpp"
#include "config.hpp"

namespace PackageMan
{
class Repository
{
public:
    virtual ~Repository() = default;

    [[nodiscard]] virtual std::optional<std::vector<std::string>> getDependencies(
        std::string_view name) const = 0;
};

class ApkRepository : public Repository
{
public:
    explicit ApkRepository(ApkIndex index) : m_index(std::move(index)) {}

    [[nodiscard]] std::optional<std::vector<std::string>> getDependencies(
        std::string_view name) const override;

private:
    ApkIndex m_index;
};

class TestRepository : public Repository
{
public:
    static std::expected<TestRepository, std::string> loadFromFile(const std::string& path);

    [[nodiscard]] std::optional<std::vector<std::string>> getDependencies(
        std::string_view name) const override;

private:
    std::unordered_map<std::string, std::vector<std::string>> m_graph;
};

std::expected<std::unique_ptr<Repository>, std::string> makeRepository(const Config& cfg);
} // namespace PackageMan
