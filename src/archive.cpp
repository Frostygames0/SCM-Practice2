#include "archive.hpp"

#include <expected>
#include <format>
#include <memory>
#include <string_view>

#include <archive.h>
#include <archive_entry.h>

namespace PackageMan::Archive
{
std::expected<std::string, std::string> unpackFileFromMem(std::span<const std::byte> mem,
                                                          std::string_view fileName)
{
    std::unique_ptr<archive, decltype(&archive_read_free)> handle(archive_read_new(),
                                                                  archive_read_free);

    archive_read_support_filter_gzip(handle.get());
    archive_read_support_format_tar(handle.get());

    if (archive_read_open_memory(handle.get(), mem.data(), mem.size()) != ARCHIVE_OK)
    {
        return std::unexpected{std::string{archive_error_string(handle.get())}};
    }

    archive_entry* entry{};
    while (archive_read_next_header(handle.get(), &entry) == ARCHIVE_OK)
    {
        if (std::string_view{archive_entry_pathname(entry)} == fileName)
        {
            std::string text(archive_entry_size(entry), '\0');
            archive_read_data(handle.get(), text.data(), text.size());

            return text;
        }

        archive_read_data_skip(handle.get());
    }

    return std::unexpected{std::format("{} was not found", fileName)};
}
} // namespace PackageMan::Archive