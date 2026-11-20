#include "filedownload.hpp"

#include <expected>
#include <format>
#include <memory>

#include <curl/curl.h>

namespace PackageMan::Web
{
std::expected<std::vector<std::byte>, std::string> downloadFrom(const std::string& link,
                                                                const ProgressCallback& onProgress)
{
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> handle(curl_easy_init(),
                                                               &curl_easy_cleanup);
    CURL* curl = handle.get();
    if (curl == nullptr)
    {
        return std::unexpected{"Unable to initialize curl!"};
    }

    std::vector<std::byte> downloadedBytes;

    curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
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

    if (onProgress)
    {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(
            curl,
            CURLOPT_XFERINFOFUNCTION,
            +[](void* userp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t, curl_off_t) -> int {
                try
                {
                    (*static_cast<const ProgressCallback*>(userp))(static_cast<std::uint64_t>(dlnow),
                                                             static_cast<std::uint64_t>(dltotal));
                }
                catch (...)
                {
                    return 1;
                }

                return 0;
            });
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &onProgress);
    }

    if (CURLcode code = curl_easy_perform(curl); code != CURLE_OK)
    {
        return std::unexpected{std::format("Download failed: {}", curl_easy_strerror(code))};
    }

    return downloadedBytes;
}
} // namespace PackageMan::Web
