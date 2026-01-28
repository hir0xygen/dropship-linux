// Linux privilege escalation implementation using pkexec

#include "privileges.h"
#include "platform.h"

#if DROPSHIP_LINUX

#include <unistd.h>
#include <cstdlib>
#include <array>
#include <memory>

namespace platform::privileges {

bool isRoot() {
    return geteuid() == 0;
}

std::filesystem::path getExecutablePath() {
    // Read the symlink at /proc/self/exe
    try {
        return std::filesystem::read_symlink("/proc/self/exe");
    } catch (...) {
        return {};
    }
}

bool isPkexecAvailable() {
    // Check if pkexec exists and is executable
    return std::system("which pkexec > /dev/null 2>&1") == 0;
}

bool restartWithPkexec() {
    if (!isPkexecAvailable()) {
        return false;
    }
    
    auto exePath = getExecutablePath();
    if (exePath.empty()) {
        return false;
    }
    
    // Build the pkexec command
    std::string command = "pkexec \"" + exePath.string() + "\"";
    
    // Execute pkexec - this replaces the current process on success
    // We use system() and then exit, as execl would not show the polkit dialog
    int result = std::system(command.c_str());
    
    // If we get here, either pkexec failed or the elevated process finished
    // Exit with the result code
    std::exit(result >> 8);
    
    return false; // Never reached
}

} // namespace platform::privileges

#endif // DROPSHIP_LINUX
