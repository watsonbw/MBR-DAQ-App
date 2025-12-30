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

using namespace std::chrono;

#define GL_WHITE 1.0, 1.0, 1.0, 1.0f

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

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawMainMenuBar();

    if (ImGui::Begin("Baja Telemetry Dashboard")) {
      ImGui::Text("Vehicle Status: %s", m_IsLogging ? "LOGGING" : "IDLE");

      // A simple button to toggle logging
      if (ImGui::Button(m_IsLogging ? "Stop Logging" : "Start Logging")) {
        m_IsLogging = !m_IsLogging;
      }

      ImGui::Separator();
      ImGui::Text("Application FPS: %.1f", io.Framerate);

      ImGui::End();
    }

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(m_Window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    // Clear screen
    glClearColor(GL_WHITE);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_Window);
  }
}

void GUI::drawMainMenuBar() {
  if (ImGui::BeginMainMenuBar()) {
    auto now = system_clock::now();

    auto time_now = system_clock::to_time_t(now);
    auto *lt = std::localtime(&time_now);

    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    ImGui::Text("%02d:%02d:%02d.%03lld", lt->tm_hour, lt->tm_min, lt->tm_sec,
                ms.count());

    ImGui::Separator();

    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Exit")) {
        glfwSetWindowShouldClose(m_Window, true);
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}
