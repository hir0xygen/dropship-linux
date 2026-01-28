// Linux firewall implementation using iptables
// Based on proven approach from ngtk4 project

#include "firewall.h"
#include "../platform.h"

#if DROPSHIP_LINUX

#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <array>
#include <memory>

namespace platform::firewall {

namespace {
    // Chain prefix for dropship rules
    constexpr const char* CHAIN_PREFIX = "DROPSHIP_";
    
    // Execute a shell command and return success/failure
    bool executeCommand(const std::string& cmd) {
        int result = std::system(cmd.c_str());
        return result == 0;
    }
    
    // Execute a command and capture output
    std::string executeCommandWithOutput(const std::string& cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) {
            return "";
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }
    
    // Check if running as root
    bool isRoot() {
        return geteuid() == 0;
    }
}

bool initialize() {
    // Check if iptables is available
    if (!executeCommand("which iptables > /dev/null 2>&1")) {
        return false;
    }
    return true;
}

void shutdown() {
    // Nothing to clean up
}

bool isFirewallEnabled() {
    // On Linux, iptables is always "enabled" if available
    // Check if we can list rules (indicates proper access)
    return executeCommand("iptables -L -n > /dev/null 2>&1");
}

std::vector<FirewallRule> getRulesInGroup(const std::string& group) {
    std::vector<FirewallRule> rules;
    
    // List iptables rules and filter by our chain prefix + group
    std::string chainName = CHAIN_PREFIX + group;
    std::string output = executeCommandWithOutput("iptables -L " + chainName + " -n 2>/dev/null");
    
    // Parse output to extract rules
    // TODO: Implement proper parsing
    
    return rules;
}

bool createRule(const FirewallRule& rule) {
    if (!isRoot()) {
        return false;
    }
    
    std::string chainName = CHAIN_PREFIX + rule.group;
    
    // Create chain if it doesn't exist
    executeCommand("iptables -N " + chainName + " 2>/dev/null");
    
    // Add jump to our chain from OUTPUT
    executeCommand("iptables -C OUTPUT -j " + chainName + " 2>/dev/null || "
                   "iptables -A OUTPUT -j " + chainName);
    
    // Add rules for each blocked address
    for (const auto& addr : rule.blocked_addresses) {
        std::string cmd = "iptables -A " + chainName + " -d " + addr + " -j DROP";
        if (!executeCommand(cmd)) {
            return false;
        }
    }
    
    return true;
}

bool setRuleAddresses(const std::string& name, const std::vector<std::string>& addresses) {
    if (!isRoot()) {
        return false;
    }
    
    std::string chainName = CHAIN_PREFIX + name;
    
    // Flush existing rules in the chain
    executeCommand("iptables -F " + chainName + " 2>/dev/null");
    
    // Add new rules
    for (const auto& addr : addresses) {
        std::string cmd = "iptables -A " + chainName + " -d " + addr + " -j DROP";
        if (!executeCommand(cmd)) {
            return false;
        }
    }
    
    return true;
}

bool setRuleEnabled(const std::string& name, bool enabled) {
    if (!isRoot()) {
        return false;
    }
    
    std::string chainName = CHAIN_PREFIX + name;
    
    if (enabled) {
        // Add jump to chain if not present
        return executeCommand("iptables -C OUTPUT -j " + chainName + " 2>/dev/null || "
                              "iptables -A OUTPUT -j " + chainName);
    } else {
        // Remove jump to chain
        return executeCommand("iptables -D OUTPUT -j " + chainName + " 2>/dev/null");
    }
}

bool deleteRule(const std::string& name) {
    if (!isRoot()) {
        return false;
    }
    
    std::string chainName = CHAIN_PREFIX + name;
    
    // Remove jump from OUTPUT
    executeCommand("iptables -D OUTPUT -j " + chainName + " 2>/dev/null");
    
    // Flush and delete the chain
    executeCommand("iptables -F " + chainName + " 2>/dev/null");
    executeCommand("iptables -X " + chainName + " 2>/dev/null");
    
    return true;
}

void forEachRuleInGroup(const std::string& group, 
                        std::function<void(const FirewallRule&)> callback) {
    auto rules = getRulesInGroup(group);
    for (const auto& rule : rules) {
        callback(rule);
    }
}

} // namespace platform::firewall

#endif // DROPSHIP_LINUX
