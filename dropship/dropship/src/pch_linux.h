#pragma once

// Linux precompiled header
// Mirrors pch.h but without Windows-specific includes

#define half_pi 1.57079632679

// C++20 features
#include <iostream>
#include <format>

// Provide println compatibility for compilers that don't have <print>
namespace dropship {
    template<typename... Args>
    void println(std::format_string<Args...> fmt, Args&&... args) {
        std::cout << std::format(fmt, std::forward<Args>(args)...) << '\n';
    }
    inline void println() {
        std::cout << '\n';
    }
}

#include <functional>
#include <optional>
#include <future>
#include <chrono>
#include <filesystem>
#include <memory>
#include <cstdio>

using namespace std::chrono_literals;

// data structures
#include <string>
#include <vector>
#include <set>
#include <unordered_map>

// imgui
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui-docking/imgui.h"
#include "imgui-docking/imgui_internal.h"

// json
#include "json/json.hpp"

// Linux-specific
#include <unistd.h>
#include <sys/types.h>
