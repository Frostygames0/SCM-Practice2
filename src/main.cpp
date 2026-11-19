#include <iostream>

#include <curl/curl.h>

#include "config.hpp"
#include "graph.hpp"
#include "render.hpp"
#include "repository.hpp"

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
        std::cerr << "Failed to initialize curl!\n";
        return 1;
    }

    auto repo = PackageMan::makeRepository(cfg);
    curl_global_cleanup();
    if (!repo)
    {
        std::cerr << "Error: " << repo.error() << '\n';
        return 1;
    }

    auto graph = PackageMan::buildGraph(**repo, cfg.packageName);

    auto mermaid = PackageMan::toMermaid(graph);
    std::cout << mermaid << '\n';

    if (auto rendered = PackageMan::renderMermaid(mermaid, cfg.outputImgName); !rendered)
    {
        std::cerr << "Error: " << rendered.error() << '\n';
        return 1;
    }
    std::cout << "Saved: " << cfg.outputImgName << '\n';

    return 0;
}
