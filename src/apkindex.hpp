#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace PackageMan
{
struct ApkPackage
{
    std::string name;                   // P
    std::string version;                // V
    std::string arch;                   // A
    std::string description;            // T
    std::string url;                    // U
    std::string license;                // L
    std::string origin;                 // o
    std::string maintainer;             // m
    std::string checksum;               // C
    std::uint64_t size = 0;             // S
    std::uint64_t installedSize = 0;    // I
    std::uint64_t buildTime = 0;        // t
    std::vector<std::string> depends;   // D
    std::vector<std::string> provides;  // p
    std::vector<std::string> installIf; // i

    friend std::ostream& operator<<(std::ostream& str, const ApkPackage& pkg);
};

class ApkIndex
{
public:
    explicit ApkIndex(std::vector<ApkPackage> packages) : m_packages(std::move(packages)) {}

    static ApkIndex parseFromString(const std::string& str);

    [[nodiscard]] const ApkPackage* get(std::string_view name) const;

private:
    std::vector<ApkPackage> m_packages;
};

} // namespace PackageMan
