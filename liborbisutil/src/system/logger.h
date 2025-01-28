#pragma once

#include <vector>
#include <string>

#include "string.h"

#include <fmt/format.h>

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

			// Log a message with the specified level
			template <typename... Args>
			void log(level lvl, std::string format, Args... args);
		}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
		template<typename ...Args>
		void internal::log(level lvl, std::string format, Args ...args)
		{
			try
			{
				std::string color = get_color(lvl);
				std::string prefix = get_prefix(lvl);

				auto size = static_cast<size_t>(std::snprintf(nullptr, 0, format.data(), args...) + 1);

				std::unique_ptr<char[]> buffer(new char[size]);
				snprintf(buffer.get(), size, format.c_str(), args...);

				// Format the message using string::format
				std::string full_str = "[" + color + std::string(prefix) + "\033[0m] " + buffer.get();

				printf("%s", full_str.c_str());
			}
			catch (std::exception& e)
			{
				printf("Failed to format string: %s\n", e.what());
			}
		}
#pragma clang diagnostic pop

		// Log a message with the specified level
		template <typename... Args>
		void info(std::string format, Args... args)
		{
			internal::log(internal::info, format, args...);
		}

		template <typename... Args>
		void success(std::string format, Args... args)
		{
			internal::log(internal::success, format, args...);
		}

		template <typename... Args>
		void warning(std::string format, Args... args)
		{
			internal::log(internal::warning, format, args...);
		}
		template <typename... Args>
		void error(std::string format, Args... args)
		{
			internal::log(internal::error, format, args...);
		}

		template <typename... Args>
		void debug(std::string format, Args... args)
		{
			internal::log(internal::debug, format, args...);
		}

		template <typename... Args>
		void verbose(std::string format, Args... args)
		{
			internal::log(internal::verbose, format, args...);
		}

		// Log a message with the specified level
		template <typename... Args>
		void log(internal::level lvl, std::string format, Args... args)
		{
			internal::log(lvl, format, args...);
		}
	}
}