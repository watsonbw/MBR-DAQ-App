#pragma once

#include <mutex>
#include <sstream>
#include <string>

#include <spdlog/spdlog.h>

class Log {
  public:
    static void                                    Init();
    inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
    static std::string                             GetStreamedLogs();

  private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    static std::ostringstream              s_OSS;
    static std::mutex                      s_ErrorMutex;
};

#define LOG_TRACE(...) Log::GetCoreLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...) Log::GetCoreLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) Log::GetCoreLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) Log::GetCoreLogger()->critical(__VA_ARGS__)
