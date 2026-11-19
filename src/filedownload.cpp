#include "filedownload.hpp"

#include <curl/curl.h>
#include <curl/easy.h>
#include <expected>
#include <format>

namespace PackageMan::Web
{
std::expected<std::vector<std::byte>, std::string> downloadFrom(const std::string& link)
{
    // TODO WRAP IN RAII TO PREVENT LEAKS
    CURL* curl = curl_easy_init();
    if (curl == nullptr)
    {
        return std::unexpected{"Unable to initialize curl!"};
    }

    std::vector<std::byte> downloadedBytes;

    curl_easy_setopt(curl, CURLOPT_URL, link.data());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        +[](char* contents, size_t size, size_t nmemb, void* userp) -> std::size_t {
            try
            {
                size_t totalBytes = size * nmemb;

                auto* buffer = static_cast<std::vector<std::byte>*>(userp);
                auto* data = reinterpret_cast<std::byte*>(contents); // NOLINT
                buffer->insert(buffer->end(), data, data + totalBytes);
                return totalBytes;
            }
            catch (...)
            {
                return 0;
            }
        });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &downloadedBytes);

    if (CURLcode code = curl_easy_perform(curl); code != CURLE_OK)
    {
        return std::unexpected{std::format("Download failed: {}", curl_easy_strerror(code))};
    }

    curl_easy_cleanup(curl);

    return downloadedBytes;
}
} // namespace PackageMan::Web