#include "apkindex.hpp"

#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include "utils/string.hpp"

namespace PackageMan
{
namespace
{
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
        case 'A':
            pkg.arch = value;
            break;
        case 'T':
            pkg.description = value;
            break;
        case 'U':
            pkg.url = value;
            break;
        case 'L':
            pkg.license = value;
            break;
        case 'o':
            pkg.origin = value;
            break;
        case 'm':
            pkg.maintainer = value;
            break;
        case 'C':
            pkg.checksum = value;
            break;
        case 'S':
            pkg.size = std::stoull(std::string(value));
            break;
        case 'I':
            pkg.installedSize = std::stoull(std::string(value));
            break;
        case 't':
            pkg.buildTime = std::stoull(std::string(value));
            break;
        case 'D':
            pkg.depends = StringUtils::split<std::string>(value, ' ');
            break;
        case 'p':
            pkg.provides = StringUtils::split<std::string>(value, ' ');
            break;
        case 'i':
            pkg.installIf = StringUtils::split<std::string>(value, ' ');
            break;
        default:
            break; // ignore unknown keys
    }
}
} // namespace

std::ostream& operator<<(std::ostream& str, const ApkPackage& pkg)
{
    str << "Name:           " << pkg.name << '\n'
        << "Version:        " << pkg.version << '\n'
        << "Arch:           " << pkg.arch << '\n'
        << "Description:    " << pkg.description << '\n'
        << "URL:            " << pkg.url << '\n'
        << "License:        " << pkg.license << '\n'
        << "Origin:         " << pkg.origin << '\n'
        << "Maintainer:     " << pkg.maintainer << '\n'
        << "Checksum:       " << pkg.checksum << '\n'
        << "Size:           " << pkg.size << '\n'
        << "Installed size: " << pkg.installedSize << '\n'
        << "Build time:     " << pkg.buildTime << '\n'
        << "Depends:        " << StringUtils::joinStrings(pkg.depends, " ") << '\n'
        << "Provides:       " << StringUtils::joinStrings(pkg.provides, " ") << '\n'
        << "Install if:     " << StringUtils::joinStrings(pkg.installIf, " ") << '\n';

    return str;
}

ApkIndex ApkIndex::parseFromString(const std::string& str)
{
    std::vector<ApkPackage> packages;

    ApkPackage current;
    bool hasData = false;

    StringUtils::splitLazy(
        str,
        '\n',
        [&](std::string_view line) {
            if (line.empty())
            {
                if (hasData)
                {
                    packages.push_back(std::move(current));
                    current = {};
                    hasData = false;
                }
                return;
            }
            if (line.size() >= 2 && line[1] == ':')
            {
                applyField(current, line[0], line.substr(2));
                hasData = true;
            }
        },
        false);

    if (hasData)
    {
        packages.push_back(std::move(current));
    }

    return ApkIndex{packages};
}

const ApkPackage* ApkIndex::get(std::string_view name) const
{
    auto it = std::find_if(m_packages.begin(), m_packages.end(), [&](const ApkPackage& pkg) {
        return pkg.name == name;
    });

    if (it != m_packages.end())
    {
        return &*it;
    }

    return nullptr;
}
} // namespace PackageMan