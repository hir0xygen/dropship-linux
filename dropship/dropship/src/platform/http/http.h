#pragma once

#include <string>
#include <optional>
#include <functional>
#include <filesystem>

namespace platform::http {

// Download text content from a URL
std::optional<std::string> downloadText(const std::string& url);

// Download a file from a URL with optional progress callback
// Progress callback receives (bytes_downloaded, total_bytes)
bool downloadFile(const std::string& url, 
                  const std::filesystem::path& destination,
                  std::function<void(size_t, size_t)> progress = nullptr);

} // namespace platform::http
