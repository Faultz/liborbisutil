# Logger Module

Colored console logging with file and line information.

## Log Levels

- `info` - General information (default color)
- `success` - Success messages (green)
- `warning` - Warning messages (yellow)
- `error` - Error messages (red)
- `debug` - Debug information
- `verbose` - Verbose output

## Usage

Use the provided macros for automatic file/line capture:

```cpp
#include <liborbisutil.h>

// Info level logging
liborbisutil::logger::info(__FILE__, __LINE__, "Player joined: %s\n", playerName);

// Success
liborbisutil::logger::success(__FILE__, __LINE__, "Level loaded successfully\n");

// Warning
liborbisutil::logger::warning(__FILE__, __LINE__, "Low memory: %d MB remaining\n", memMB);

// Error
liborbisutil::logger::error(__FILE__, __LINE__, "Failed to load texture: %s\n", path);

// Debug (typically stripped in release)
liborbisutil::logger::debug(__FILE__, __LINE__, "Position: %.2f, %.2f, %.2f\n", x, y, z);

// Verbose
liborbisutil::logger::verbose(__FILE__, __LINE__, "Frame time: %.3f ms\n", frameTime);
```

## Output Format

```
[COLOR PREFIX/filename.cpp:123] Your message here
```

Example output:
```
[INFO/main.cpp:45] Game initialized
[SUCCESS/loader.cpp:102] Assets loaded: 256 textures
[WARNING/memory.cpp:78] Memory pool 80% full
[ERROR/network.cpp:34] Connection timed out
```

## Printf-style Formatting

All logging functions support printf-style format strings:

```cpp
liborbisutil::logger::info(__FILE__, __LINE__, 
    "Player %s at position (%.1f, %.1f, %.1f) with %d HP\n",
    name, x, y, z, health);
```

## Direct Log Function

For dynamic log levels:

```cpp
liborbisutil::logger::log(
    liborbisutil::logger::internal::warning,
    __FILE__, __LINE__,
    "Dynamic log level message\n"
);
```
