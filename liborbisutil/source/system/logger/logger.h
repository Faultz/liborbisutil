#pragma once

// Logger - standard includes provided by stdafx.h
// Depends on: string utilities

#include "../string/string.h"

namespace liborbisutil
{
	namespace logger
	{
		namespace internal
		{
			enum level
			{
				info,
				success,
				warning,
				error,
				debug,
				verbose,
				max
			};

			const char* get_color(level lvl);
			const char* get_prefix(level lvl);

			// Log a message with the specified level, file, and line
			template <typename... Args>
			void log(level lvl, const char* file, int line, std::string format, Args... args);
		}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
		template<typename ...Args>
		void internal::log(level lvl, const char* file, int line, std::string format, Args ...args)
		{
			try
			{
				std::string color = get_color(lvl);
				std::string prefix = get_prefix(lvl);

				auto size = static_cast<size_t>(std::snprintf(nullptr, 0, format.data(), args...) + 1);

				std::unique_ptr<char[]> buffer(new char[size]);
				snprintf(buffer.get(), size, format.c_str(), args...);

				// Format the message to include file and line information
				std::string full_str = "[" + color + std::string(prefix) + "/" + std::string(file) + ":" + std::to_string(line) + "\033[0m] " + buffer.get();

				printf("%s", full_str.c_str());
			}
			catch (std::exception& e)
			{
				printf("Failed to format string: %s\n", e.what());
			}
		}
#pragma clang diagnostic pop

		// Log a message with the specified level, file, and line
		template <typename... Args>
		void info(const char* file, int line, std::string format, Args... args)
		{
			internal::log(internal::info, file, line, format, args...);
		}

		template <typename... Args>
		void success(const char* file, int line, std::string format, Args... args)
		{
			internal::log(internal::success, file, line, format, args...);
		}

		template <typename... Args>
		void warning(const char* file, int line, std::string format, Args... args)
		{
			internal::log(internal::warning, file, line, format, args...);
		}

		template <typename... Args>
		void error(const char* file, int line, std::string format, Args... args)
		{
			internal::log(internal::error, file, line, format, args...);
		}

		template <typename... Args>
		void debug(const char* file, int line, std::string format, Args... args)
		{
			internal::log(internal::debug, file, line, format, args...);
		}

		template <typename... Args>
		void verbose(const char* file, int line, std::string format, Args... args)
		{
			internal::log(internal::verbose, file, line, format, args...);
		}

		// Log a message with the specified level, file, and line
		template <typename... Args>
		void log(internal::level lvl, const char* file, int line, std::string format, Args... args)
		{
			internal::log(lvl, file, line, format, args...);
		}
	}
}

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

// Macros to simplify logging calls
#define LOG_INFO(format, ...) liborbisutil::logger::info(__FILENAME__, __LINE__, format, ##__VA_ARGS__)
#define LOG_SUCCESS(format, ...) liborbisutil::logger::success(__FILENAME__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) liborbisutil::logger::warning(__FILENAME__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) liborbisutil::logger::error(__FILENAME__, __LINE__, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) liborbisutil::logger::debug(__FILENAME__, __LINE__, format, ##__VA_ARGS__)
#define LOG_VERBOSE(format, ...) liborbisutil::logger::verbose(__FILENAME__, __LINE__, format, ##__VA_ARGS__)
#define LOG(level, format, ...) liborbisutil::logger::log(level, __FILENAME__, __LINE__, format, ##__VA_ARGS__)