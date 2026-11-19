#include <iostream>

#include <archive.h>
#include <archive_entry.h>

#include <curl/curl.h>
#include <curl/easy.h>

#include <CLI11/CLI11.hpp>

#include "apkindex.hpp"
#include "archive.hpp"
#include "config.hpp"
#include "filedownload.hpp"
#include "utils/string.hpp"

int main(int argc, char** argv)
{
    auto res = PackageMan::parseFromCommandLine(argc, argv);
    if (!res)
    {
        return res.error();
    }

    const PackageMan::Config cfg = *std::move(res);

    if (auto valRes = PackageMan::validateConfig(cfg); !valRes)
    {
        std::cerr << "Error: " << valRes.error() << '\n';
        return 1;
    }

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
    {
        std::cerr << "Failed to initialize curl!";
        return 1;
    }

    if (cfg.testMode)
    {
        std::cerr << "Test mode is currently not supported!";
        return 1;
    }

    auto downloadRes = PackageMan::Web::downloadFrom(cfg.repoPath);
    if (!downloadRes)
    {
        std::cerr << downloadRes.error();
        return 1;
    }

    auto unarchiveRes = PackageMan::Archive::readFromMemory(*downloadRes);
    if (!unarchiveRes)
    {
        std::cerr << unarchiveRes.error();
        return 1;
    }

    auto index = PackageMan::ApkIndex::parseFromString(*unarchiveRes);
    if (const auto* ptr = index.get(cfg.packageName); ptr != nullptr)
    {
        std::cout << PackageMan::StringUtils::joinStrings(ptr->depends, " ") << '\n';
    }

    return 0;
}
