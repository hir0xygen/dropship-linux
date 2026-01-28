#pragma once

#include <string>
#include <vector>
#include <optional>
#include <functional>

namespace platform::firewall {

struct FirewallRule {
    std::string name;
    std::string group;
    std::string description;
    std::vector<std::string> blocked_addresses; // CIDR notation
    bool enabled = false;
};

// Initialize firewall subsystem
bool initialize();

// Shutdown firewall subsystem
void shutdown();

// Check if firewall/iptables is available and enabled
bool isFirewallEnabled();

// Get all rules in a specific group
std::vector<FirewallRule> getRulesInGroup(const std::string& group);

// Create a new firewall rule
bool createRule(const FirewallRule& rule);

// Update an existing rule's blocked addresses
bool setRuleAddresses(const std::string& name, const std::vector<std::string>& addresses);

// Enable or disable a rule
bool setRuleEnabled(const std::string& name, bool enabled);

// Delete a rule by name
bool deleteRule(const std::string& name);

// Iterate over rules in a group with a callback
void forEachRuleInGroup(const std::string& group, 
                        std::function<void(const FirewallRule&)> callback);

} // namespace platform::firewall
