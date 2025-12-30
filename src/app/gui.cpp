#include <chrono>
#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include "app/gui.hpp"
#include "app/style.hpp"

using namespace std::chrono;

static const char* GLSL_VERSION = "#version 130";
static const char* WINDOW_TITLE = "Michigan Baja Racing - Data Suite";

GUI::GUI(std::shared_ptr<AppContext> ctx) : m_Context{ctx} {
    InitGLFW();
    InitImGui();
}

GUI::~GUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void GUI::Launch() {
    while (!glfwWindowShouldClose(m_Window)) {
        glfwPollEvents();
        Update();
    }

    m_Context->ShouldExit = true;
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

void GUI::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);

    m_Fonts        = LoadFonts();
    auto& io       = ImGui::GetIO();
    io.FontDefault = m_Fonts.Regular;

    SetDarkThemeColors();
}

void GUI::StartFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::Update() {
    StartFrame();

    DrawMainMenuBar();
    DrawDashboard();

    EndFrame();
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

void GUI::DrawMainMenuBar() {
    ImGui::PushFont(m_Fonts.Regular, 36.0f);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(m_Window, true); }
            ImGui::EndMenu();
        }

        ImGui::Separator();

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

        ImGui::EndMainMenuBar();
    }

    ImGui::PopFont();
}

void GUI::DrawDashboard() {
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::Begin("Baja Telemetry Dashboard")) {
        bool logging = m_Context->IsLogging;
        ImGui::Text("Vehicle Status: %s", logging ? "LOGGING" : "IDLE");

        // A simple button to toggle logging
        if (ImGui::Button(logging ? "Stop Logging" : "Start Logging")) {
            m_Context->IsLogging = !logging;
        }

        ImGui::Separator();
        ImGui::Text("Application FPS: %.1f", io.Framerate);
    }

    // FYI: Windows will crash if they dont have an end at the end of their scope!
    ImGui::End();
}
