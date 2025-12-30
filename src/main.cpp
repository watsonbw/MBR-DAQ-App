#include <iostream>
#include <vector>

// ImGui Headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// GLFW
#include <GLFW/glfw3.h>

int main() {
    // --- 1. Initialize GLFW ---
    if (!glfwInit()) return -1;

    // Use OpenGL 3.0 or higher
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Michigan Baja Racing - Data Suite", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable V-Sync

    // --- 2. Initialize Dear ImGui ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark(); // You can also use StyleColorsClassic()

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // --- 3. App State (Telemetry Simulation) ---
    float rpm = 0.0f;
    std::vector<float> rpm_history;
    bool is_logging = false;

    // --- 4. Main Loop ---
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- 5. Build the UI ---
        {
            ImGui::Begin("Baja Telemetry Dashboard");

            ImGui::Text("Vehicle Status: %s", is_logging ? "LOGGING" : "IDLE");
            
            // A simple button to toggle logging
            if (ImGui::Button(is_logging ? "Stop Logging" : "Start Logging")) {
                is_logging = !is_logging;
            }

            ImGui::SliderFloat("Simulated RPM", &rpm, 0.0f, 4000.0f);

            // Record history for the graph
            rpm_history.push_back(rpm);
            if (rpm_history.size() > 100) rpm_history.erase(rpm_history.begin());

            // Simple Plot
            ImGui::PlotLines("Engine RPM", rpm_history.data(), (int)rpm_history.size(), 0, nullptr, 0.0f, 4000.0f, ImVec2(0, 80));

            ImGui::Separator();
            ImGui::Text("Application FPS: %.1f", io.Framerate);
            
            ImGui::End();
        }

        // --- 6. Rendering ---
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        
        // Clear screen with a "Baja Blue" color
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // --- 7. Cleanup ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}