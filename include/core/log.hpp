#pragma once

#include <functional>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace mbr {

using log_fn_t = std::function<void(spdlog::level::level_enum, std::string_view)>;

class log_t {
  public:
    log_t();
    ~log_t() = default;

    [[nodiscard]] log_fn_t    get_log_fn();
    [[nodiscard]] std::string get_streamed_logs();

  private:
    std::shared_ptr<spdlog::logger> logger_;
    std::ostringstream              oss_;
    std::mutex                      mutex_;
};

template <typename... Args>
void log_trace(const log_fn_t& log, std::string_view fmt, Args&&... args) {
    if (log) {
        if constexpr (sizeof...(Args) == 0) {
            log(spdlog::level::trace, fmt);
        } else {
            log(spdlog::level::trace, fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...));
        }
    }
}

template <typename... Args>
void log_info(const log_fn_t& log, std::string_view fmt, Args&&... args) {
    if (log) {
        if constexpr (sizeof...(Args) == 0) {
            log(spdlog::level::info, fmt);
        } else {
            log(spdlog::level::info, fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...));
        }
    }
}

template <typename... Args>
void log_warn(const log_fn_t& log, std::string_view fmt, Args&&... args) {
    if (log) {
        if constexpr (sizeof...(Args) == 0) {
            log(spdlog::level::warn, fmt);
        } else {
            log(spdlog::level::warn, fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...));
        }
    }
}

template <typename... Args>
void log_error(const log_fn_t& log, std::string_view fmt, Args&&... args) {
    if (log) {
        if constexpr (sizeof...(Args) == 0) {
            log(spdlog::level::err, fmt);
        } else {
            log(spdlog::level::err, fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...));
        }
    }
}

template <typename... Args>
void log_critical(const log_fn_t& log, std::string_view fmt, Args&&... args) {
    if (log) {
        if constexpr (sizeof...(Args) == 0) {
            log(spdlog::level::critical, fmt);
        } else {
            log(spdlog::level::critical,
                fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...));
        }
    }
}

} // namespace mbr
