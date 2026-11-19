#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace PackageMan
{
struct ApkPackage
{
    std::string name;                  // P
    std::string version;               // V
    std::vector<std::string> depends;  // D (normalized: no versions, no conflicts)
    std::vector<std::string> provides; // p (normalized: no versions)
};

class ApkIndex
{
public:
    explicit ApkIndex(std::vector<ApkPackage> packages);

    static ApkIndex parseFromString(const std::string& str);

    [[nodiscard]] const ApkPackage* get(std::string_view name) const;

    [[nodiscard]] const ApkPackage* resolve(std::string_view name) const;

    [[nodiscard]] const std::vector<ApkPackage>& packages() const { return m_packages; }

private:
    void buildIndices();

    std::vector<ApkPackage> m_packages;
    std::unordered_map<std::string, std::size_t> m_byName;
    std::unordered_map<std::string, std::size_t> m_byProvides;
};

} // namespace PackageMan
