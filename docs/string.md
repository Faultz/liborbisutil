# String Module

String manipulation and formatting utilities.

## Case Conversion

```cpp
#include <liborbisutil.h>

std::string lower = liborbisutil::string::to_lower("Hello World");
// Result: "hello world"

std::string upper = liborbisutil::string::to_upper("Hello World");
// Result: "HELLO WORLD"
```

## Wide String Conversion

```cpp
std::wstring wide = liborbisutil::string::to_wide("Hello");
std::string narrow = liborbisutil::string::to_narrow(L"Hello");
```

## String Testing

```cpp
// Check prefix/suffix
bool starts = liborbisutil::string::begins_with("HelloWorld", "Hello");  // true
bool ends = liborbisutil::string::ends_with("file.txt", ".txt");         // true

// Contains substring
bool has = liborbisutil::string::contains("Hello World", "World");       // true
bool hasInsensitive = liborbisutil::string::contains("HELLO", "hello", false);  // true

// Check if contains any of multiple substrings
std::vector<std::string> patterns = {"error", "warning", "fatal"};
bool hasAny = liborbisutil::string::contains(logLine, patterns);

// Character type checks
bool numeric = liborbisutil::string::is_numeric("12345");      // true
bool integer = liborbisutil::string::is_integer("-42");        // true
bool alpha = liborbisutil::string::is_alphabetic("Hello");     // true
```

## String Manipulation

```cpp
// Reverse
std::string rev = liborbisutil::string::reverse("Hello");  // "olleH"

// Replace
std::string replaced = liborbisutil::string::replace("Hello World", "World", "PS4");
// Result: "Hello PS4"

// Erase
std::string erased = liborbisutil::string::erase("Hello World", " World");
// Result: "Hello"
```

## Splitting and Joining

```cpp
// Split string
std::vector<std::string> parts = liborbisutil::string::split("a,b,c", ",");
// Result: {"a", "b", "c"}

// Tokenize (single char delimiter)
std::vector<std::string> tokens = liborbisutil::string::tokenize("a b c", ' ');

// Join vector
std::string joined = liborbisutil::string::join({"a", "b", "c"}, ", ");
// Result: "a, b, c"

// Concatenate from index
std::string concat = liborbisutil::string::concatenate({"a", "b", "c"}, '-', 1);
// Result: "b-c"
```

## Formatting

```cpp
// Printf-style formatting
std::string msg = liborbisutil::string::format("Player %s: %d HP", name, health);

// Shorthand alias
std::string msg = liborbisutil::string::va("Value: %d", value);

// Wide string formatting
std::wstring wmsg = liborbisutil::string::format(L"Score: %d", score);
```

## Type Conversion

```cpp
// To string
std::string str = liborbisutil::string::to_string(42);
std::string str = liborbisutil::string::to_string(3.14f);

// From string
int value = liborbisutil::string::from_string<int>("42");
float fval = liborbisutil::string::from_string<float>("3.14");
```

## Array Conversion

```cpp
// Vector to C-style array
const char** arr = liborbisutil::string::vector_to_pointer_array(stringVec);

// C-style array to vector
std::vector<std::string> vec = liborbisutil::string::pointer_array_to_vector(arr, count);
```
