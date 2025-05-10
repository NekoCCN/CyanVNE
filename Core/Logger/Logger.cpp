#include "Logger.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

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
			std::string formatted_time = std::format("{:_%Y-%m-%d_%H:%M:%S}", std::chrono::system_clock::now());
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
			std::string formatted_time = std::format("{:_%Y-%m-%d_%H:%M:%S}", std::chrono::system_clock::now());
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
