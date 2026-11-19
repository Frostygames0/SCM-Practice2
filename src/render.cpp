#include "render.hpp"

#include <chrono>
#include <format>
#include <fstream>
#include <vector>

#include <reproc++/run.hpp>

#include "tempfile.hpp"

namespace PackageMan
{
std::expected<void, std::string> renderMermaid(const std::string& mermaid,
                                               const std::string& outputPath)
{
    TempFile input(".mmd");
    {
        std::ofstream file(input.path());
        if (!(file << mermaid))
        {
            return std::unexpected(std::format("cannot write {}", input.path().string()));
        }
    }

#ifdef _WIN32
    const std::string mmdc = "mmdc.cmd";
#else
    const std::string mmdc = "mmdc";
#endif

    std::vector<std::string> args{mmdc, "-i", input.path().string(), "-o", outputPath};

    reproc::options options;
    options.redirect.parent = true;
    options.deadline = reproc::milliseconds(std::chrono::minutes(2));

    auto [status, ec] = reproc::run(args, options);
    if (ec)
    {
        return std::unexpected(std::format("failed to run {}: {}", mmdc, ec.message()));
    }
    if (status != 0)
    {
        return std::unexpected(std::format("{} exited with code {}", mmdc, status));
    }
    return {};
}
} // namespace PackageMan
