#include <chrono>
#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include "app/fonts/OpenSans.hpp"

#include "app/gui.hpp"
#include "app/style.hpp"

using namespace std::chrono;

static const char* GLSL_VERSION = "#version 130";
static const char* WINDOW_TITLE = "Michigan Baja Racing - Data Suite";

GUI::GUI() : m_IsConnected{false}, m_IsLogging{false} {
    InitGLFW();
    InitImGUI();
}

GUI::~GUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

bool GUI::InitGLFW() {
    if (!glfwInit()) {
        std::cerr << "I couldn't init GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    m_Window = glfwCreateWindow(1920, 1080, WINDOW_TITLE, nullptr, nullptr);

    if (!m_Window) {
        glfwTerminate();
        std::cerr << "I couldn't create the window" << std::endl;
        return false;
    }

    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);

    return true;
}

void GUI::InitImGUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);

    ImGuiIO&     io = ImGui::GetIO();
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(OpenSansRegular_ttf),
                                   OpenSansRegular_ttf_size,
                                   DEFAULT_FONT_SIZE,
                                   &cfg);
}

void GUI::StartFrame() {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::EndFrame() {
    ImGui::Render();
    glfwGetFramebufferSize(m_Window, &m_WindowData.DisplayWidth, &m_WindowData.DisplayHeight);
    glViewport(0, 0, m_WindowData.DisplayWidth, m_WindowData.DisplayHeight);

    glClearColor(GL_WHITE);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_Window);
}

void GUI::Launch() {
    while (!glfwWindowShouldClose(m_Window)) {
        StartFrame();

        DrawMainMenuBar();
        DrawDashboard();

        EndFrame();
    }
}

void GUI::DrawMainMenuBar() {
    // PLACEHOLDER LOGIC
    if (ImGui::BeginMainMenuBar()) {
        auto now = system_clock::now();

        auto    time_now = system_clock::to_time_t(now);
        std::tm lt{};
#ifdef _WIN32
        localtime_s(&lt, &time_now);
#else
        localtime_r(&time_now, &lt);
#endif

        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        ImGui::Text("%02d:%02d:%02d.%03lld", lt.tm_hour, lt.tm_min, lt.tm_sec, ms.count());

        ImGui::Separator();

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(m_Window, true); }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void GUI::DrawDashboard() {
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::Begin("Baja Telemetry Dashboard")) {
        ImGui::Text("Vehicle Status: %s", m_IsLogging ? "LOGGING" : "IDLE");

        // A simple button to toggle logging
        if (ImGui::Button(m_IsLogging ? "Stop Logging" : "Start Logging")) {
            m_IsLogging = !m_IsLogging;
        }

        ImGui::Separator();
        ImGui::Text("Application FPS: %.1f", io.Framerate);
    }

    // FYI: Windows will crash if they dont have an end at the end of their scope!
    ImGui::End();
}
