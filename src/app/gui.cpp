#include <chrono>
#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <implot.h>

#include <GLFW/glfw3.h>

#include "app/gui.hpp"
#include "app/style.hpp"

#include "app/pages/home.hpp"
#include "app/pages/rpm.hpp"
#include "app/pages/shock.hpp"
#include "app/pages/view.hpp"

using namespace std::chrono;

static const char* GLSL_VERSION = "#version 130";
static const char* WINDOW_TITLE = "Michigan Baja Racing - Data Suite";

GUI::GUI(std::shared_ptr<AppContext> ctx) : m_Context{ctx} {
    InitGLFW();
    InitImGui();

    ChangePage(PageType::HOME);
}

GUI::~GUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();

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

    ImPlot::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);

    m_Context->Fonts = LoadFonts();
    auto& io         = ImGui::GetIO();
    io.FontDefault   = m_Context->Fonts.Regular;

    SetDarkThemeColors();
}

void GUI::ChangePage(PageType type) {
    if (m_CurrentPage) { m_CurrentPage->OnExit(); }

    switch (type) {
    case PageType::HOME:
        m_CurrentPage = std::make_unique<HomePage>(m_Context);
        break;
    case PageType::RPM:
        m_CurrentPage = std::make_unique<RPMPage>(m_Context);
        break;
    case PageType::SHOCK:
        m_CurrentPage = std::make_unique<ShockPage>(m_Context);
        break;
    case PageType::VIEW:
        m_CurrentPage = std::make_unique<ViewPage>(m_Context);
        break;
    }

    if (m_CurrentPage) { m_CurrentPage->OnEnter(); }
    m_Context->CurrentPageType = type;
}

void GUI::StartFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::Update() {
    StartFrame();

    DrawMainMenuBar();

    if (m_CurrentPage) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);

        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        m_CurrentPage->Update();
        ImGui::PopStyleVar(2);
    }

    EndFrame();
}

void GUI::EndFrame() {
    ImGui::Render();
    glfwGetFramebufferSize(m_Window, &m_WindowData.DisplayWidth, &m_WindowData.DisplayHeight);
    glViewport(0, 0, m_WindowData.DisplayWidth, m_WindowData.DisplayHeight);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_Window);
}

void GUI::DrawMainMenuBar() {
    ImGui::PushFont(m_Context->Fonts.Regular, 36.0f);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Home")) { ChangePage(PageType::HOME); }
            if (ImGui::MenuItem("RPM")) { ChangePage(PageType::RPM); }
            if (ImGui::MenuItem("Shock")) { ChangePage(PageType::SHOCK); }
            if (ImGui::MenuItem("View")) { ChangePage(PageType::VIEW); }
            if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(m_Window, true); }

            ImGui::EndMenu();
        }

        ImGui::Separator();

        ImGui::Text(PageTypeString(m_Context->CurrentPageType));

        ImGui::Separator();

        if (ImGui::Button("Sync Time")) {}

        ImGui::Separator();

        if (ImGui::Button("Restart Connection")) {}

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
