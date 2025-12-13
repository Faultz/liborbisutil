#pragma once

// String utilities - standard includes provided by stdafx.h

namespace liborbisutil
{
	namespace string
	{
		std::string to_lower(const std::string& str);
		std::string to_upper(const std::string& str);

		std::wstring to_wide(const std::string& str);
		std::string to_narrow(const std::wstring& str);

		bool begins_with(const std::string& str, const std::string& prefix);
		bool ends_with(const std::string& str, const std::string& suffix);

		bool contains(const std::string& str, const std::string& sub, const bool sensitive = true);
		bool contains(const std::string& str, const std::vector<std::string>& subs, const bool sensitive = true);

		std::string join(const std::vector<std::string>& strings, const std::string& delimiter = "\0");
		std::vector<std::string> tokenize(const std::string& str, char delimiter = '\0');
		std::vector<std::string> split(const std::string& str, const std::string& delimiter = "\0");
		std::string concatenate(const std::vector<std::string>& strings, char delimiter, size_t idx = 0);

		std::string reverse(const std::string& str);
		std::string replace(const std::string& str, const std::string& from, const std::string& to);
		std::string erase(const std::string& str, const std::string& sub);

		const char** vector_to_pointer_array(const std::vector<std::string>& strings);
		std::vector<std::string> pointer_array_to_vector(const char* const* strings, std::size_t count);

		bool is_numeric(const std::string& str);
		bool is_integer(const std::string& str);
		bool is_alphabetic(const std::string& str);


		template <typename T>
		std::string to_string(const T& value)
		{
			std::ostringstream oss;
			oss << value;
			return oss.str();
		}

		template <typename T>
		T from_string(const std::string& str)
		{
			std::istringstream iss(str);
			T value;
			iss >> value;
			return value;
		}

		template <typename ...Args>
		std::string format(std::string fmt, Args... args)
		{
			try
			{
				size_t size = std::snprintf(nullptr, 0, fmt.c_str(), args...) + 1;
				std::unique_ptr<char[]> buffer(new char[size]);
				std::snprintf(buffer.get(), size, fmt.c_str(), args...);
				return std::string(buffer.get(), buffer.get() + size - 1);
			}
			catch (std::exception& e)
			{
				printf("Failed to format string: %s\n", e.what());
				return {};
			}
		}

		template<typename ...Args>
		std::string va(std::string fmt, Args... args)
		{
			return format(fmt, args...);
		}

		template <typename ...Args>
		std::wstring format(std::wstring fmt, Args... args)
		{
			try
			{
				size_t size = std::swprintf(nullptr, 0, fmt, args...) + 1;
				std::unique_ptr<wchar_t[]> buffer(new wchar_t[size]);
				std::swprintf(buffer.get(), size, fmt, args...);
				return std::wstring(buffer.get(), buffer.get() + size - 1);
			}
			catch (std::exception e)
			{
				printf("Failed to format string: %s\n", e.what());
				return {};
			}
		}
	}
}