#pragma once

#include <string>
#include <filesystem>

namespace platform::privileges {

// Check if running with root privileges
bool isRoot();

// Get the path to the current executable
std::filesystem::path getExecutablePath();

// Restart the application with elevated privileges using pkexec
// Returns false if restart failed, does not return on success
bool restartWithPkexec();

// Check if pkexec is available on the system
bool isPkexecAvailable();

} // namespace platform::privileges
