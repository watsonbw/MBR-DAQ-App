#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <imgui.h>

struct AppContext;

class TextUtils {
  public:
    explicit TextUtils(const std::shared_ptr<AppContext>& ctx) : m_Context{ctx} {}
    ~TextUtils() = default;

    void DrawStartLoggingButton();

    // Appends the buf data to the filepath, clearing the buf on success.
    void        DrawDataDownloadButton(const std::vector<std::string>& raw_lines, std::string& buf);
    static bool DrawInputBox(const char*                label,
                             std::string&               buf,
                             std::optional<const char*> hint        = std::nullopt,
                             float                      width_scale = 200.0F,
                             ImGuiInputTextFlags        flags       = 0);
    static void DrawDataLog(const std::vector<std::string>& raw_lines);

  private:
    std::shared_ptr<AppContext> m_Context;
};
