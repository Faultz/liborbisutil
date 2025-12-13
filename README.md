# liborbisutil

A comprehensive utility library for PlayStation 4 (ORBIS) development, providing essential system-level functionality, networking, hooking, math utilities, and more.

## Features

- **System Utilities** - Memory management, file system operations, logging, threading, and more
- **Hooking** - Function hooking and detouring with MinHook
- **Networking** - HTTP client and socket operations
- **Math** - Vector2, Vector3, Vector4, and Quaternion classes
- **Audio** - WAV file playback and audio player
- **Input** - Controller/pad input handling

## Modules

| Module | Description |
|--------|-------------|
| [logger](docs/logger.md) | Colored console logging with file/line info |
| [memory](docs/memory.md) | Memory operations and symbol resolution |
| [memory_allocators](docs/memory_allocators.md) | Custom allocators and heap management |
| [filesystem](docs/filesystem.md) | File I/O and directory iteration |
| [string](docs/string.md) | String manipulation utilities |
| [thread](docs/thread.md) | Threading and thread pool |
| [resolve](docs/resolve.md) | Module and symbol resolution |
| [patcher](docs/patcher.md) | Runtime memory patching |
| [pad](docs/pad.md) | Controller input handling |
| [audio](docs/audio.md) | Audio playback system |
| [hooking](docs/hooking.md) | Function hooking framework |
| [http](docs/http.md) | HTTP client |
| [sockets](docs/sockets.md) | TCP/UDP sockets |
| [math](docs/math.md) | Vector and quaternion math |

## Usage

Include the main header to access all functionality:

```cpp
#include <liborbisutil.h>
```

## Dependencies

- PlayStation 4 SDK
- MinHook library
- fmtlib

## License

See LICENSE file for details.
