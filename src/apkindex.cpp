#include "apkindex.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "utils/string.hpp"

namespace PackageMan
{
namespace
{
// "!foo" -> nullopt, "busybox>=1.36" -> "busybox", "so:libz.so.1=1.3" -> "so:libz.so.1"
std::optional<std::string> normalizeToken(std::string_view tok)
{
    if (tok.empty() || tok.front() == '!')
    {
        return std::nullopt;
    }
    return std::string{tok.substr(0, tok.find_first_of("=<>~"))};
}

std::vector<std::string> parseList(std::string_view value)
{
    std::vector<std::string> out;
    StringUtils::splitLazy(value, ' ', [&](std::string_view tok) {
        if (auto name = normalizeToken(tok); name && !name->empty())
        {
            out.push_back(std::move(*name));
        }
    });
    return out;
}

void applyField(ApkPackage& pkg, char key, std::string_view value)
{
    switch (key)
    {
        case 'P':
            pkg.name = value;
            break;
        case 'V':
            pkg.version = value;
            break;
        case 'D':
            pkg.depends = parseList(value);
            break;
        case 'p':
            pkg.provides = parseList(value);
            break;
        default:
            break; // ignore unknown keys
    }
}

template <typename Map>
const ApkPackage* findIn(const Map& map, const std::vector<ApkPackage>& pkgs, std::string_view name)
{
    if (auto it = map.find(std::string{name}); it != map.end())
    {
        return &pkgs[it->second];
    }
    return nullptr;
}
} // namespace

ApkIndex::ApkIndex(std::vector<ApkPackage> packages) : m_packages(std::move(packages))
{
    buildIndices();
}

ApkIndex ApkIndex::parseFromString(const std::string& str)
{
    std::vector<ApkPackage> packages;

    ApkPackage current;
    bool hasData = false;

    auto flush = [&] {
        if (hasData)
        {
            packages.push_back(std::move(current));
            current = {};
            hasData = false;
        }
    };

    StringUtils::splitLazy(
        str,
        '\n',
        [&](std::string_view line) {
            if (line.empty())
            {
                flush();
                return;
            }
            if (line.size() >= 2 && line[1] == ':')
            {
                applyField(current, line[0], line.substr(2));
                hasData = true;
            }
        },
        false);

    flush();

    return ApkIndex{std::move(packages)};
}

const ApkPackage* ApkIndex::get(std::string_view name) const
{
    return findIn(m_byName, m_packages, name);
}

const ApkPackage* ApkIndex::resolve(std::string_view name) const
{
    if (const auto* pkg = get(name))
    {
        return pkg;
    }
    return findIn(m_byProvides, m_packages, name);
}

void ApkIndex::buildIndices()
{
    m_byName.clear();
    m_byProvides.clear();

    for (std::size_t i = 0; i < m_packages.size(); ++i)
    {
        const auto& pkg = m_packages[i];
        m_byName.emplace(pkg.name, i);
        for (const auto& p : pkg.provides)
        {
            m_byProvides.emplace(p, i); // first provider wins
        }
    }
}
} // namespace PackageMan
