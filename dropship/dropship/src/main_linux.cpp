// Linux entry point for Dropship
// Uses GLFW + OpenGL3 instead of Win32 + DirectX11

#include "pch_linux.h"

#include <cstdio>
#include <clocale>
#include <memory>
#include <optional>
#include <future>
#include <vector>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <iostream>

// ImGui backends
#include "imgui-docking/imgui_impl_glfw.h"
#include "imgui-docking/imgui_impl_opengl3.h"

// OpenGL/GLFW
#include <GLFW/glfw3.h>

// Platform
#include "platform/platform.h"
#include "platform/firewall/firewall.h"
#include "platform/http/http.h"
#include "platform/privileges.h"

// Fonts
ImFont* font_title = nullptr;
ImFont* font_subtitle = nullptr;
ImFont* font_text = nullptr;

bool dashboard_open = true;
bool show_privilege_dialog = false;
static std::string http_test_result;
static bool show_http_result = false;

// GLFW error callback
static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Find resource directory (for fonts, images, etc.)
std::filesystem::path getResourcePath() {
    // Check AppImage path first
    if (const char* appdir = std::getenv("APPDIR")) {
        auto path = std::filesystem::path(appdir) / "usr" / "share" / "dropship";
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    
    // Check XDG data home
    if (const char* xdg_data = std::getenv("XDG_DATA_HOME")) {
        auto path = std::filesystem::path(xdg_data) / "dropship";
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    
    // Check ~/.local/share/dropship
    if (const char* home = std::getenv("HOME")) {
        auto path = std::filesystem::path(home) / ".local" / "share" / "dropship";
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    
    // Check /usr/share/dropship
    auto sysPath = std::filesystem::path("/usr/share/dropship");
    if (std::filesystem::exists(sysPath)) {
        return sysPath;
    }
    
    // Fallback to current directory
    return std::filesystem::current_path();
}

// Temporary: render a placeholder UI until the real UI is ported
void renderPlaceholderUI(bool* p_open) {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                                     ImGuiWindowFlags_AlwaysAutoResize;
    
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Dropship - Linux Port (WIP)", p_open, window_flags)) {
        ImGui::PushFont(font_title);
        ImGui::Text("Dropship");
        ImGui::PopFont();
        
        ImGui::Separator();
        
        ImGui::Text("Linux port is under development.");
        ImGui::Text("This is a placeholder UI.");
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Privilege status
        if (platform::privileges::isRoot()) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Running as root");
        } else {
            ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Not running as root (firewall won't work)");
            if (ImGui::Button("Restart with elevated privileges")) {
                show_privilege_dialog = true;
            }
        }
        
        // Firewall status
        ImGui::Spacing();
        if (platform::firewall::isFirewallEnabled()) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "iptables: Available");
        } else {
            ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "iptables: Not available");
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Test buttons
        if (ImGui::Button("Test HTTP Download")) {
            auto result = platform::http::downloadText("https://httpbin.org/get");
            if (result) {
                http_test_result = "Success!\n\n" + result.value().substr(0, 200) + "...";
            } else {
                http_test_result = "Failed to download.";
            }
            show_http_result = true;
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Exit")) {
            *p_open = false;
        }
    }
    ImGui::End();
    
    // Privilege escalation dialog
    if (show_privilege_dialog) {
        ImGui::OpenPopup("Restart as Root?");
        show_privilege_dialog = false;
    }
    
    if (ImGui::BeginPopupModal("Restart as Root?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Dropship requires root privileges to manage firewall rules.");
        ImGui::Text("Would you like to restart with elevated privileges?");
        ImGui::Spacing();
        
        if (!platform::privileges::isPkexecAvailable()) {
            ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "pkexec is not available on this system.");
            ImGui::Text("Please run: sudo ./dropship");
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (platform::privileges::isPkexecAvailable()) {
            if (ImGui::Button("Restart as Root", ImVec2(150, 0))) {
                ImGui::CloseCurrentPopup();
                // This will restart the app with pkexec
                platform::privileges::restartWithPkexec();
            }
            ImGui::SameLine();
        }
        
        if (ImGui::Button("Cancel", ImVec2(100, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // HTTP test result popup
    if (show_http_result) {
        ImGui::OpenPopup("HTTP Test Result");
        show_http_result = false;
    }

    if (ImGui::BeginPopupModal("HTTP Test Result", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", http_test_result.c_str());
        ImGui::Spacing();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    setlocale(LC_ALL, "en_US.UTF-8");
    
    // Check privileges
    if (!platform::privileges::isRoot()) {
        std::cerr << "Warning: Dropship requires root privileges for firewall management.\n";
        std::cerr << "Please restart with: pkexec " << platform::privileges::getExecutablePath().string() << "\n";
        // Continue anyway - will show dialog in UI
    }
    
    // Initialize GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }
    
    // GL 3.3 + GLSL 330
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Dropship", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return 1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    io.ConfigDragClickToInputText = true;
    
    // Setup style
    ImGui::StyleColorsLight();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    // Load fonts
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = true;
    
    auto resourcePath = getResourcePath();
    auto fontsPath = resourcePath / "fonts";
    
    // Try to load fonts, fall back to default if not found
    auto robotoPath = fontsPath / "Roboto-Regular.ttf";
    
    if (std::filesystem::exists(robotoPath)) {
        font_text = io.Fonts->AddFontFromFileTTF(robotoPath.c_str(), 18.0f, &font_cfg);
        font_title = io.Fonts->AddFontFromFileTTF(robotoPath.c_str(), 32.0f, &font_cfg);
        font_subtitle = io.Fonts->AddFontFromFileTTF(robotoPath.c_str(), 14.0f, &font_cfg);
    }
    
    // If fonts not found, use default
    if (!font_text) {
        font_text = io.Fonts->AddFontDefault();
        font_title = font_text;
        font_subtitle = font_text;
    }
    
    // Disable ini/log files
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    
    ImVec4 clear_color = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    
    // Initialize platform firewall
    if (!platform::firewall::initialize()) {
        std::cerr << "Warning: Failed to initialize firewall subsystem\n";
    }
    
    // Main loop
    while (!glfwWindowShouldClose(window) && dashboard_open) {
        glfwPollEvents();
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Render placeholder UI
        renderPlaceholderUI(&dashboard_open);
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, 
                     clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
        
        // Handle escape key
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            dashboard_open = false;
        }
    }
    
    // Cleanup platform
    platform::firewall::shutdown();
    
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    // Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
