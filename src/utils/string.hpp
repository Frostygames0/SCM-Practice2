#pragma once

#include <concepts>
#include <cstddef>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace PackageMan::StringUtils
{

template <std::ranges::range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, std::string_view>
[[nodiscard]] constexpr std::string joinStrings(const R& parts, std::string_view delim)
{
    if (parts.empty())
    {
        return {};
    }

    std::size_t totalSize{};
    for (const auto& part : parts)
    {
        totalSize += part.size();
    }
    totalSize += delim.size() * (parts.size() - 1);

    std::string result;
    result.reserve(totalSize);

    result += parts[0];
    for (std::size_t i{1}; i < parts.size(); ++i)
    {
        result.append(delim);
        result.append(parts[i]);
    }

    return result;
}

template <std::invocable<std::string_view> F>
constexpr void splitLazy(std::string_view str, char delim, F&& func, bool skipEmpty = true) // NOLINT
{
    std::size_t start{};
    std::size_t end = str.find(delim);

    while (end != std::string_view::npos)
    {
        auto part = str.substr(start, end - start);
        if (!skipEmpty || !part.empty())
        {
            func(part);
        }

        start = end + 1;
        end = str.find(delim, start);
    }

    auto lastPart = str.substr(start);
    if (!skipEmpty || !lastPart.empty())
    {
        func(str.substr(start));
    }
}

template <std::constructible_from<std::string_view> T = std::string_view>
[[nodiscard]] constexpr std::vector<T> split(std::string_view str,
                                             char delim,
                                             bool skipEmpty = true)
{
    std::vector<T> result;
    splitLazy(str, delim, [&](std::string_view part) { result.emplace_back(part); }, skipEmpty);

    return result;
}

template <typename T = std::string_view>
    requires std::constructible_from<T, std::string_view> && std::default_initializable<T>
constexpr T trim(std::string_view sv)
{
    constexpr const char* spaces = " \t\n\r\f\v";

    std::size_t start = sv.find_first_not_of(spaces);
    if (start == std::string_view::npos)
    {
        return T{};
    }

    std::size_t end = sv.find_last_not_of(spaces);

    return T(sv.substr(start, end - start + 1));
}
}; // namespace PackageMan::StringUtils