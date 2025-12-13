# Logger Module

Colored console logging with file and line information.

## Log Levels

- `info` - General information (default color)
- `success` - Success messages (green)
- `warning` - Warning messages (yellow)
- `error` - Error messages (red)
- `debug` - Debug information
- `verbose` - Verbose output

## Macros (Recommended)

Use the provided macros for convenient logging with automatic file/line capture:

```cpp
#include <liborbisutil.h>

// Info level
LOG_INFO("Player joined: %s\n", playerName);

// Success
LOG_SUCCESS("Level loaded successfully\n");

// Warning
LOG_WARNING("Low memory: %d MB remaining\n", memMB);

// Error
LOG_ERROR("Failed to load texture: %s\n", path);

// Debug
LOG_DEBUG("Position: %.2f, %.2f, %.2f\n", x, y, z);

// Verbose
LOG_VERBOSE("Frame time: %.3f ms\n", frameTime);

// Generic with custom level
LOG(liborbisutil::logger::internal::warning, "Custom level message\n");
```

## Macro Reference

| Macro | Description |
|-------|-------------|
| `LOG_INFO(format, ...)` | Log info message |
| `LOG_SUCCESS(format, ...)` | Log success message |
| `LOG_WARNING(format, ...)` | Log warning message |
| `LOG_ERROR(format, ...)` | Log error message |
| `LOG_DEBUG(format, ...)` | Log debug message |
| `LOG_VERBOSE(format, ...)` | Log verbose message |
| `LOG(level, format, ...)` | Log with custom level |
| `__FILENAME__` | Extracts just the filename from `__FILE__` |

## Direct Function Usage

For more control, use the functions directly:

```cpp
#include <liborbisutil.h>

// Using __FILENAME__ macro (just filename, not full path)
liborbisutil::logger::info(__FILENAME__, __LINE__, "Player joined: %s\n", playerName);

// Or with full path
liborbisutil::logger::info(__FILE__, __LINE__, "Player joined: %s\n", playerName);

// Other levels
liborbisutil::logger::success(__FILENAME__, __LINE__, "Level loaded\n");
liborbisutil::logger::warning(__FILENAME__, __LINE__, "Low memory\n");
liborbisutil::logger::error(__FILENAME__, __LINE__, "Failed to load\n");
liborbisutil::logger::debug(__FILENAME__, __LINE__, "Debug info\n");
liborbisutil::logger::verbose(__FILENAME__, __LINE__, "Verbose output\n");
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

All logging functions and macros support printf-style format strings:

```cpp
LOG_INFO("Player %s at position (%.1f, %.1f, %.1f) with %d HP\n",
    name, x, y, z, health);
```

## Log Levels Enum

```cpp
namespace liborbisutil::logger::internal {
    enum level {
        info,
        success,
        warning,
        error,
        debug,
        verbose,
        max
    };
}
```
