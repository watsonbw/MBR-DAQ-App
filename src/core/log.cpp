#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "core/log.hpp"

namespace mbr {

log_t::log_t() {
    std::vector<spdlog::sink_ptr> log_sinks;
    size_t                        log_idx = 0;
#ifdef LOGGING
    log_sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    log_sinks.emplace_back(
        std::make_shared<spdlog::sinks::basic_file_sink_mt>("MBR-DAQ-Output.log", true));

    log_sinks[log_idx++]->set_pattern("%^[%T] %n: %v%$");
    log_sinks[log_idx++]->set_pattern("[%T] [%l] %n: %v");
#endif
    log_sinks.emplace_back(std::make_shared<spdlog::sinks::ostream_sink_mt>(oss_));
    log_sinks[log_idx++]->set_pattern("[%l]: %v");

    logger_ = std::make_shared<spdlog::logger>("MBR-DAQ-DEBUG", log_sinks.begin(), log_sinks.end());

#ifdef LOGGING
    logger_->set_level(spdlog::level::trace);
    logger_->flush_on(spdlog::level::trace);
#else
    logger_->set_level(spdlog::level::info);
    logger_->flush_on(spdlog::level::info);
#endif
}

std::string log_t::get_streamed_logs() {
    const std::scoped_lock<std::mutex> lock{mutex_};
    return oss_.str();
}

log_fn_t log_t::get_log_fn() {
    return [this](spdlog::level::level_enum level, std::string_view msg) {
        if (logger_) { logger_->log(level, msg); }
    };
}

} // namespace mbr
