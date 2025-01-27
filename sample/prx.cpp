
#include "liborbisutil.h"

// We need to provide an export to force the expected stub library to be generated
__declspec (dllexport) void dummy()
{

}

extern "C" 
{
	int module_start(size_t argc, const void* args)
	{
		auto str = liborbisutil::string::to_lower("Hello, World!");
		printf("Lowercase: %s\n", str.c_str());

		str = liborbisutil::string::to_upper("Hello, World!");
		printf("Uppercase: %s\n", str.c_str());

		auto wide = liborbisutil::string::to_wide("Hello, World!");
		auto narrow = liborbisutil::string::to_narrow(wide);

		printf("Wide: %ls\n", wide.c_str());
		printf("Narrow: %s\n", narrow.c_str());

		auto result = liborbisutil::string::begins_with("Hello, World!", "Hello");
		printf("Begins with: %s\n", result ? "true" : "false");

		result = liborbisutil::string::ends_with("Hello, World!", "World!");
		printf("Ends with: %s\n", result ? "true" : "false");

		result = liborbisutil::string::contains("Hello, World!", "World");
		printf("Contains: %s\n", result ? "true" : "false");

		result = liborbisutil::string::contains("Hello, World!", "world", false);
		printf("Contains (case-insensitive): %s\n", result ? "true" : "false");

		auto strings = liborbisutil::string::split("Hello, World!", ",");
		for (auto& str : strings)
		{
			printf("Split: %s\n", str.c_str());
		}

		str = liborbisutil::string::join(strings, " ");
		printf("Join: %s\n", str.c_str());

		str = liborbisutil::string::reverse("Hello, World!");
		printf("Reverse: %s\n", str.c_str());

		str = liborbisutil::string::replace("Hello, World!", "World", "Orbis");
		printf("Replace: %s\n", str.c_str());

		str = liborbisutil::string::erase("Hello, World!", "World");
		printf("Erase: %s\n", str.c_str());

		auto pointer_array = liborbisutil::string::vector_to_pointer_array(strings);
		auto vector = liborbisutil::string::pointer_array_to_vector(pointer_array);

		for (auto& str : vector)
		{
			printf("Pointer array: %s\n", str.c_str());
		}

		result = liborbisutil::string::is_numeric("123");
		printf("Is numeric: %s\n", result ? "true" : "false");

		result = liborbisutil::string::is_integer("123");
		printf("Is integer: %s\n", result ? "true" : "false");

		result = liborbisutil::string::is_alphabetic("Hello");
		printf("Is alphabetic: %s\n", result ? "true" : "false");

		return 0;
	}

	int module_stop(size_t argc, const void* args)
	{
		return 0;
	}
}