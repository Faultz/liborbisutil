# Resolve Module

Module loading, symbol resolution, and address utilities.

## Size Literals

Convenient size literal operators:

```cpp
size_t kb = 64_KB;   // 65,536 bytes
size_t mb = 16_MB;   // 16,777,216 bytes
size_t gb = 2_GB;    // 2,147,483,648 bytes
```

## Process Information

```cpp
#include <liborbisutil.h>

// Get process base address
uintptr_t base = liborbisutil::resolve::get_process_base();
```

## Module Functions

### Loading/Unloading

```cpp
// Load by module ID
liborbisutil::resolve::load_module(SCE_SYSMODULE_HTTP);

// Load by path
liborbisutil::resolve::load_module("/app0/modules/mylib.prx");

// Unload
liborbisutil::resolve::unload_module(SCE_SYSMODULE_HTTP);
liborbisutil::resolve::unload_module("/app0/modules/mylib.prx");

// Check if loaded
if (liborbisutil::resolve::is_loaded(SCE_SYSMODULE_HTTP)) {
    // Module is loaded
}
```

### Module Information

```cpp
// Get module info
SceKernelModuleInfo info = liborbisutil::resolve::get_module_info("libScePad.sprx");

// Get module size
size_t size = liborbisutil::resolve::get_module_size("libScePad.sprx");
```

## Address Resolution

### Get Export Address

```cpp
// Get function address from module export
uintptr_t addr = liborbisutil::resolve::get_export_address(
    "libScePad.sprx",
    "scePadRead"
);

// Cast to function pointer
typedef int (*scePadRead_t)(int, ScePadData*, int);
auto scePadRead = reinterpret_cast<scePadRead_t>(addr);
```

### Get Module Address

```cpp
// Get address within a module (base + offset)
auto funcPtr = liborbisutil::resolve::get_module_address<void*>(
    "libScePad.sprx",
    0x1234
);

// Get export by name (same as get_export_address)
auto func = liborbisutil::resolve::get_module_address<scePadRead_t>(
    "libScePad.sprx",
    "scePadRead"
);
```

### Absolute Address

```cpp
// Convert offset to absolute address (process base + offset)
auto addr = liborbisutil::resolve::get_absolute_address(0x12345678);

// With custom base
auto addr = liborbisutil::resolve::get_absolute_address<void*>(0x1000, customBase);
```

## Instruction Reading

```cpp
// Read instruction bytes at address
uint64_t instruction = liborbisutil::resolve::read_instruction(
    address,
    4,    // opcode size
    8     // total length to read
);
```

## User Information

```cpp
// Get foreground user ID
uint64_t userId = liborbisutil::resolve::get_foreground_user();
```

## Example: Dynamic Function Calling

```cpp
// Dynamically call a module function
auto my_func = liborbisutil::resolve::get_module_address<int(*)(int, int)>(
    "libMyModule.sprx",
    "myFunction"
);

if (my_func) {
    int result = my_func(10, 20);
}
```
