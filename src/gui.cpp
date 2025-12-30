#include <chrono>
#include <iostream>
#include <vector>

// ImGui Headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// GLFW
#include <GLFW/glfw3.h>

#include "gui.hpp"

GUI::GUI() : m_isConnected{false}, m_IsLogging{false} {
  if (!glfwInit()) {
    throw std::runtime_error("I couldn't init GLFW");
  }

  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  m_Window = glfwCreateWindow(1920, 1080, "Michigan Baja Racing - Data Suite",
                              nullptr, nullptr);

  if (!m_Window) {
    glfwTerminate();
    throw std::runtime_error("I couldn't create the window");
  }
  glfwMakeContextCurrent(m_Window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);
}

GUI::~GUI() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(m_Window);
  glfwTerminate();
}

void GUI::launch() {
  ImGuiIO &io = ImGui::GetIO();

  while (!glfwWindowShouldClose(m_Window)) {
    glfwPollEvents();

    // Start ImGui Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // --- 5. Build the UI ---
    {
      ImGui::Begin("Baja Telemetry Dashboard");

      ImGui::Text("Vehicle Status: %s", m_IsLogging ? "LOGGING" : "IDLE");

      // A simple button to toggle logging
      if (ImGui::Button(m_IsLogging ? "Stop Logging" : "Start Logging")) {
        m_IsLogging = !m_IsLogging;
      }

      ImGui::SliderFloat("Simulated RPM", &m_RPM, 0.0f, 4000.0f);

      // Record history for the graph
      m_RPMHistory.push_back(m_RPM);
      if (m_RPMHistory.size() > 100)
        m_RPMHistory.erase(m_RPMHistory.begin());

      // Simple Plot
      ImGui::PlotLines("Engine RPM", m_RPMHistory.data(),
                       (int)m_RPMHistory.size(), 0, nullptr, 0.0f, 4000.0f,
                       ImVec2(0, 80));

      ImGui::Separator();
      ImGui::Text("Application FPS: %.1f", io.Framerate);

      ImGui::End();
    }

    // --- 6. Rendering ---
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(m_Window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    // Clear screen
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_Window);
  }
}
