// Linux HTTP implementation using libcurl

#include "http.h"
#include "../platform.h"

#if DROPSHIP_LINUX

#include <curl/curl.h>
#include <fstream>

namespace platform::http {

namespace {
    // Callback for writing received data to a string
    size_t writeStringCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        std::string* str = static_cast<std::string*>(userp);
        str->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }
    
    // Callback for writing received data to a file
    size_t writeFileCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        std::ofstream* file = static_cast<std::ofstream*>(userp);
        size_t totalSize = size * nmemb;
        file->write(static_cast<char*>(contents), totalSize);
        return totalSize;
    }
    
    // Progress callback wrapper
    struct ProgressData {
        std::function<void(size_t, size_t)> callback;
    };
    
    int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                         curl_off_t /*ultotal*/, curl_off_t /*ulnow*/) {
        ProgressData* data = static_cast<ProgressData*>(clientp);
        if (data && data->callback && dltotal > 0) {
            data->callback(static_cast<size_t>(dlnow), static_cast<size_t>(dltotal));
        }
        return 0;
    }
}

std::optional<std::string> downloadText(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return std::nullopt;
    }
    
    std::string response;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeStringCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Dropship/1.0");
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        return std::nullopt;
    }
    
    return response;
}

bool downloadFile(const std::string& url, 
                  const std::filesystem::path& destination,
                  std::function<void(size_t, size_t)> progress) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    
    std::ofstream file(destination, std::ios::binary);
    if (!file) {
        curl_easy_cleanup(curl);
        return false;
    }
    
    ProgressData progressData{progress};
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFileCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Dropship/1.0");
    
    if (progress) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progressData);
    }
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    file.close();
    
    if (res != CURLE_OK) {
        std::filesystem::remove(destination);
        return false;
    }
    
    return true;
}

} // namespace platform::http

#endif // DROPSHIP_LINUX
