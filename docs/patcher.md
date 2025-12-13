# Patcher Module

Runtime memory patching system for modifying code at runtime.

## Creating Patches

### NOP Patch

Replace instructions with NOPs:

```cpp
#include <liborbisutil.h>

// NOP by module + offset
auto* patch = liborbisutil::patcher::nop(
    "skip_check",           // Patch name
    "eboot.bin",            // Module
    0x123456,               // Offset
    5                       // Size in bytes
);

// NOP by absolute address
auto* patch = liborbisutil::patcher::nop(
    "skip_check",
    absolute_address,
    5
);
```

### Custom Byte Patch

```cpp
// Patch with custom bytes
std::vector<uint8_t> newBytes = { 0x90, 0x90, 0xEB, 0x10 };

auto* patch = liborbisutil::patcher::create(
    "my_patch",
    "eboot.bin",
    0x123456,
    newBytes,
    false  // Don't apply immediately
);

// Or by absolute address
auto* patch = liborbisutil::patcher::create(
    "my_patch",
    absolute_address,
    newBytes,
    true  // Apply immediately
);

// Alternative: add() is an alias for create()
auto* patch = liborbisutil::patcher::add("patch_name", address, bytes);
```

## Managing Patches

### Enable/Disable

```cpp
// Enable patch (apply patched bytes)
liborbisutil::patcher::enable("my_patch");

// Disable patch (restore original bytes)
liborbisutil::patcher::disable("my_patch");

// Toggle patch state
liborbisutil::patcher::toggle("my_patch");

// Restore original bytes
liborbisutil::patcher::restore("my_patch");
```

### Get Patch Info

```cpp
auto* info = liborbisutil::patcher::get("my_patch");
if (info) {
    printf("Address: 0x%lx\n", info->address);
    printf("Enabled: %s\n", info->is_set() ? "yes" : "no");
}
```

### Remove Patch

```cpp
liborbisutil::patcher::remove("my_patch");
```

## Patch Info Structure

```cpp
struct info {
    uintptr_t address;              // Patch address
    std::vector<uint8_t> original;  // Original bytes
    std::vector<uint8_t> patched;   // Patched bytes
    bool set;                       // Currently applied?
    
    bool is_set();
    void enable();
    void disable();
    void toggle();
    void restore();
};
```

## Example: Skip License Check

```cpp
// Find the license check call and NOP it
liborbisutil::patcher::nop("license_check", "eboot.bin", 0x4A5B6C, 5);
liborbisutil::patcher::enable("license_check");
```

## Example: Modify Jump

```cpp
// Change conditional jump to unconditional
// Original: 74 10 (JE +0x10)
// Patched:  EB 10 (JMP +0x10)
liborbisutil::patcher::create(
    "force_jump",
    "eboot.bin",
    0x123456,
    { 0xEB },
    true
);
```

## Example: Function Return Override

```cpp
// Make function always return 1
// mov eax, 1; ret
std::vector<uint8_t> alwaysTrue = { 
    0xB8, 0x01, 0x00, 0x00, 0x00,  // mov eax, 1
    0xC3                            // ret
};

liborbisutil::patcher::create("always_success", funcAddress, alwaysTrue, true);
```
