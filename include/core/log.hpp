#pragma once

#include <mutex>
#include <sstream>
#include <string>

#include <spdlog/spdlog.h>

class Log {
  public:
    static void                                           Init();
    [[nodiscard]] static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
    [[nodiscard]] static std::string                      GetStreamedLogs();

  private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger; // NOLINT
    static std::ostringstream              s_OSS;        // NOLINT
    static std::mutex                      s_ErrorMutex; // NOLINT
};

#define LOG_TRACE(...) Log::GetCoreLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...) Log::GetCoreLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) Log::GetCoreLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) Log::GetCoreLogger()->critical(__VA_ARGS__)
