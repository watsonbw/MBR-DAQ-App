#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "core/log.hpp"

std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
std::ostringstream              Log::s_OSS;
std::mutex                      Log::s_ErrorMutex;

void Log::Init() {
    std::vector<spdlog::sink_ptr> log_sinks;
    size_t                        log_idx = 0;
#ifdef LOGGING
    log_sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    log_sinks.emplace_back(
        std::make_shared<spdlog::sinks::basic_file_sink_mt>("MBR-DAC-Output.log", true));

    log_sinks[log_idx++]->set_pattern("%^[%T] %n: %v%$");
    log_sinks[log_idx++]->set_pattern("[%T] [%l] %n: %v");
#endif
    log_sinks.emplace_back(std::make_shared<spdlog::sinks::ostream_sink_mt>(s_OSS));
    log_sinks[log_idx++]->set_pattern("[%l]: %v");

    s_CoreLogger =
        std::make_shared<spdlog::logger>("MBR-DAC-DEBUG", log_sinks.begin(), log_sinks.end());
    spdlog::register_logger(s_CoreLogger);

#ifdef LOGGING
    s_CoreLogger->set_level(spdlog::level::trace);
    s_CoreLogger->flush_on(spdlog::level::trace);
#else
    s_CoreLogger->set_level(spdlog::level::warn);
    s_CoreLogger->flush_on(spdlog::level::warn);
#endif
}

std::string Log::GetStreamedLogs() {
    const std::scoped_lock<std::mutex> lock{s_ErrorMutex};
    return s_OSS.str();
}
