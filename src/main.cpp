#include <iostream>

#include "config.hpp"

#include <CLI11/CLI11.hpp>

int main(int argc, char** argv)
{
    CLI::App app{"PackageMan: Alpine (apk) dependency graph visualizer"};
    PackageMan::Config cfg;

    app.add_option("-p,--package", cfg.packageName, "Package name")->required();
    app.add_option("-r,--repo", cfg.repoPath, "Repository URL or test file path")->required();
    app.add_flag("-t,--test", cfg.testMode, "Use test repository file");
    app.add_option("-o,--output", cfg.outputImgName, "Output PNG file name");
    CLI11_PARSE(app, argc, argv);

    auto res = PackageMan::validateConfig(cfg);
    if (!res)
    {
        std::cerr << "Error: " << res.error() << '\n';
        return 1;
    }

    std::cout << cfg;
    return 0;
}
