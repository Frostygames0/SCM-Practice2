#include "archive.hpp"

#include <archive.h>
#include <archive_entry.h>
#include <expected>

namespace PackageMan::Archive
{
std::expected<std::string, std::string> readFromMemory(std::span<const std::byte> mem)
{
    archive* a = archive_read_new();
    archive_read_support_filter_gzip(a);
    archive_read_support_format_tar(a);

    if (archive_read_open_memory(a, mem.data(), mem.size()) != ARCHIVE_OK)
    {
        return std::unexpected{std::string{archive_error_string(a)}};
    }

    archive_entry* entry{};
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
    {
        if (std::string_view{archive_entry_pathname(entry)} == "APKINDEX")
        {
            std::string text(archive_entry_size(entry), '\0');
            archive_read_data(a, text.data(), text.size());

            return text;
        }

        archive_read_data_skip(a);
    }

    archive_read_free(a);
    return std::unexpected{"APKINDEX was not found"};
}
} // namespace PackageMan::Archive