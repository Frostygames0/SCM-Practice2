#pragma once

#include <filesystem>
#include <format>
#include <random>
#include <string_view>
#include <system_error>

namespace PackageMan
{
class TempFile
{
public:
    explicit TempFile(std::string_view extension)
    {
        std::random_device rd;
        std::uniform_int_distribution<unsigned long long> dist;
        m_path = std::filesystem::temp_directory_path() /
                 std::format("packageman-{:016x}{}", dist(rd), extension);
    }

    ~TempFile()
    {
        std::error_code ec;
        std::filesystem::remove(m_path, ec);
    }

    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;
    TempFile(TempFile&&) = delete;
    TempFile& operator=(TempFile&&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const { return m_path; }

private:
    std::filesystem::path m_path;
};
} // namespace PackageMan
