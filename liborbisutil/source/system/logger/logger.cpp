#include "../../stdafx.h"
#include "logger.h"

namespace liborbisutil
{
	namespace logger
	{
		namespace internal
		{
			const char* get_color(level lvl)
			{
				switch (lvl)
				{
				case info:
					// magenta
					return "\033[35m";
				case success:
					return "\033[32m";
				case warning:
					return "\033[33m";
				case error:
					return "\033[31m";
				case debug:
					return "\033[36m";
				case verbose:
					return "\033[37m";
				default:
					return "\033[0m";
				}
			}

			// Get the prefix for the log level
			const char* get_prefix(level lvl)
			{
				switch (lvl)
				{
				case info:
					return "INFO";
				case success:
					return "SUCCESS";
				case warning:
					return "WARNING";
				case error:
					return "ERROR";
				case debug:
					return "DEBUG";
				case verbose:
					return "VERBOSE";
				default:
					return "UNKNOWN";
				}
			}
		}
	}
}