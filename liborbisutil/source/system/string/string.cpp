#include "../../stdafx.h"
#include "string.h"

namespace liborbisutil
{
	namespace string
	{
		std::string to_lower(const std::string& str)
		{
			std::string result = str;
			std::transform(result.begin(), result.end(), result.begin(), ::tolower);
			return result;
		}

		std::string to_upper(const std::string& str)
		{
			std::string result = str;
			std::transform(result.begin(), result.end(), result.begin(), ::toupper);
			return result;
		}

		std::wstring to_wide(const std::string& str)
		{
			std::wstring result;
			result.resize(str.size());
			std::copy(str.begin(), str.end(), result.begin());
			return result;
		}

		std::string to_narrow(const std::wstring& str)
		{
			std::string result;
			result.resize(str.size());
			std::copy(str.begin(), str.end(), result.begin());
			return result;
		}

		bool begins_with(const std::string& str, const std::string& prefix)
		{
			if (str.size() < prefix.size())
				return false;
			return str.compare(0, prefix.size(), prefix) == 0;
		}

		bool ends_with(const std::string& str, const std::string& suffix)
		{
			if (str.size() < suffix.size())
				return false;
			return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
		}

		bool contains(const std::string& str, const std::string& sub, const bool sensitive)
		{
			if (sensitive)
				return str.find(sub) != std::string::npos;
			else
				return to_lower(str).find(to_lower(sub)) != std::string::npos;
		}

		bool contains(const std::string& str, const std::vector<std::string>& subs, const bool sensitive)
		{
			for (auto& sub : subs)
			{
				if (contains(str, sub, sensitive))
					return true;
			}
			return false;
		}

		std::string join(const std::vector<std::string>& strings, const std::string& delimiter)
		{
			std::string result;
			for (size_t i = 0; i < strings.size(); i++)
			{
				result += strings[i];
				if (i < strings.size() - 1)
					result += delimiter;
			}
			return result;
		}

		std::vector<std::string> tokenize(const std::string& str, char delimiter)
		{
			std::vector<std::string> buffer;
			std::stringstream ss(str);

			std::string token;
			while (std::getline(ss, token, delimiter))
			{
				buffer.push_back(token);
			}

			return buffer;
		}

		std::vector<std::string> split(const std::string& str, const std::string& delimiter)
		{
			std::vector<std::string> buffer;
			size_t start = 0;
			size_t end = 0;
			while ((end = str.find(delimiter, start)) != std::string::npos)
			{
				buffer.push_back(str.substr(start, end - start));
				start = end + delimiter.size();
			}
			buffer.push_back(str.substr(start));
			return buffer;
		}

		std::string concatenate(const std::vector<std::string>& strings, char delimiter, size_t idx)
		{
			std::string result;
			for (size_t i = idx; i < strings.size(); i++)
			{
				result += strings[i];
				result += delimiter;
			}
			return result;
		}

		std::string reverse(const std::string& str)
		{
			std::string result = str;
			std::reverse(result.begin(), result.end());
			return result;
		}

		std::string replace(const std::string& str, const std::string& from, const std::string& to)
		{
			std::string result = str;
			size_t start_pos = 0;
			while ((start_pos = result.find(from, start_pos)) != std::string::npos)
			{
				result.replace(start_pos, from.length(), to);
				start_pos += to.length();
			}
			return result;
		}

		std::string erase(const std::string& str, const std::string& sub)
		{
			std::string result = str;
			size_t start_pos = 0;
			while ((start_pos = result.find(sub, start_pos)) != std::string::npos)
			{
				result.erase(start_pos, sub.length());
			}
			return result;
		}

		const char** vector_to_pointer_array(const std::vector<std::string>& strings)
		{
			std::vector<const char*> buf;
			buf.clear();

			for (auto& str : strings)
			{
				buf.push_back(str.c_str());
			}

			return buf.data();
		}

		std::vector<std::string> pointer_array_to_vector(const char* const* strings, std::size_t count)
		{
			std::vector<std::string> buf;
			buf.clear();

			for (int i = 0; i < count; i++)
			{
				if (strings[i])
				{
					buf.push_back(strings[i]);
				}
			}

			return buf;
		}

		bool is_numeric(const std::string& str)
		{
			return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
		}

		bool is_integer(const std::string& str)
		{
			if (str.empty())
				return false;
			size_t start = 0;
			if (str[0] == '+' || str[0] == '-')
				start = 1;
			return std::all_of(str.begin() + start, str.end(), ::isdigit);
		}

		bool is_alphabetic(const std::string& str)
		{
			return !str.empty() && std::all_of(str.begin(), str.end(), ::isalpha);
		}
	}
}