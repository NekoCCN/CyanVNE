#include "Logger.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

std::mutex cyanvne::core::GlobalLogger::_mutex;

std::shared_ptr<spdlog::logger> cyanvne::core::GlobalLogger::_core_logger = nullptr;
std::shared_ptr<spdlog::logger> cyanvne::core::GlobalLogger::_client_logger = nullptr;

void cyanvne::core::GlobalLogger::initUniversalCoreLogger(const LoggerConfig& config)
{
	std::lock_guard<std::mutex> lock(_mutex);

#ifdef __ANDROID__

	if (_core_logger != nullptr)
	{
		_core_logger->warn("Core logger already initialized");
		return;
	}
	_core_logger = spdlog::android_logger_mt("core", "cyanvne");
	return;

#endif

	if (_core_logger != nullptr)
	{
		_core_logger->warn("Core logger already initialized");
		return;
	}

	std::vector<spdlog::sink_ptr> sinks;

	if (config.enable_console)
	{
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(config.console_log_level);
		sinks.push_back(console_sink);
	}

	if (config.enable_file)
	{
		if (config.is_rotating)
		{
			auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(config.file_path + ".log", 1024 * 1024 * 8, 4);
			rotating_sink->set_level(config.file_log_level);
			sinks.push_back(rotating_sink);
		}
		else if (config.is_timestamped)
		{
			auto now_chrono = std::chrono::system_clock::now();
			std::time_t now_c = std::chrono::system_clock::to_time_t(now_chrono);

			// Unsafe in multithreaded environment!!!
			std::tm* tm_ptr = std::localtime(&now_c);

			std::string formatted_time;
			if (tm_ptr != nullptr)
			{
				std::stringstream ss;
				ss << std::put_time(tm_ptr, "%Y-%m-%d_%H:%M:%S");
				formatted_time = ss.str();
			}
			else
			{
				formatted_time = "";
			}

			auto timestamped_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config.file_path + formatted_time + ".log", false);
			timestamped_sink->set_level(config.file_log_level);
			sinks.push_back(timestamped_sink);
		}
		else
		{
			auto basic_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config.file_path + ".log", false);
			basic_sink->set_level(config.file_log_level);
			sinks.push_back(basic_sink);
		}
	}
	_core_logger = std::make_shared<spdlog::logger>("core", begin(sinks), end(sinks));
}

void cyanvne::core::GlobalLogger::initUniversalClientLogger(const LoggerConfig& config)
{
	std::lock_guard<std::mutex> lock(_mutex);

#ifdef __ANDROID__

	if (_client_logger != nullptr)
	{
		_client_logger->warn("Client logger already initialized");
		return;
	}
	_core_logger = spdlog::android_logger_mt("client", "cyanvne");
	return

#endif

	if (_client_logger != nullptr)
	{
		_client_logger->warn("Client logger already initialized");
		return;
	}

	std::vector<spdlog::sink_ptr> sinks;

	if (config.enable_console)
	{
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(config.console_log_level);
		sinks.push_back(console_sink);
	}

	if (config.enable_file)
	{
		if (config.is_rotating)
		{
			auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(config.file_path + ".log", 1024 * 1024 * 8, 4);
			rotating_sink->set_level(config.file_log_level);
			sinks.push_back(rotating_sink);
		}
		else if (config.is_timestamped)
		{
			auto now_chrono = std::chrono::system_clock::now();
			std::time_t now_c = std::chrono::system_clock::to_time_t(now_chrono);

			// Unsafe in multithreaded environment!!!
			std::tm* tm_ptr = std::localtime(&now_c);

			std::string formatted_time;
			if (tm_ptr != nullptr)
			{
				std::stringstream ss;
				ss << std::put_time(tm_ptr, "%Y-%m-%d_%H:%M:%S");
				formatted_time = ss.str();
			}
			else
			{
				formatted_time = "";
			}

			auto timestamped_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config.file_path + formatted_time + ".log", false);
			timestamped_sink->set_level(config.file_log_level);
			sinks.push_back(timestamped_sink);
		}
		else
		{
			auto basic_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config.file_path + ".log", false);
			basic_sink->set_level(config.file_log_level);
			sinks.push_back(basic_sink);
		}
	}

	_client_logger = std::make_shared<spdlog::logger>("client", begin(sinks), end(sinks));
}
