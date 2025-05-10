#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/android_sink.h>
#include <Core/CoreException/CoreException.h>
#include <memory>
#include <chrono>
#include <mutex>

namespace cyanvne
{
    namespace core
    {
        class GlobalLogger
        {
        private:
            static std::mutex _mutex;

            static std::shared_ptr<spdlog::logger> _core_logger;
            static std::shared_ptr<spdlog::logger> _client_logger;
        public:
            struct LoggerConfig
            {
                bool enable_console = true;
                spdlog::level::level_enum console_log_level = spdlog::level::trace;

                bool enable_file = false;
                bool is_timestamped = false;
                std::string file_path = "logs/cyanvne_log";

                bool is_rotating = true;

                spdlog::level::level_enum file_log_level = spdlog::level::trace;
            };

            GlobalLogger() = delete;
            GlobalLogger(const GlobalLogger&) = delete;
            GlobalLogger(GlobalLogger&&) = delete;
            GlobalLogger& operator=(const GlobalLogger&) = delete;
            GlobalLogger& operator=(GlobalLogger&&) = delete;
            ~GlobalLogger() = delete;

            static void initUniversalCoreLogger(const LoggerConfig& config);
            static void initUniversalClientLogger(const LoggerConfig& config);

            static const std::shared_ptr<spdlog::logger>& getCoreLogger()
            {
                if (!_core_logger)
                {
                    throw exception::coreexception::LogSystemNotInitException("Core logger not initialized");
                }
                    return _core_logger;
            }
            static const std::shared_ptr<spdlog::logger>& getClientLogger()
            {
                if (!_client_logger)
                {
                    throw exception::coreexception::LogSystemNotInitException("Client logger not initialized");
                }
                return _client_logger;
            }
        };
    }
}
