# Memory Module

Memory operations, symbol resolution, and memory region utilities.

## Symbol Class

Type-safe wrapper for accessing memory at fixed offsets from the process base.

```cpp
#include <liborbisutil.h>

// Define a symbol for a global variable
liborbisutil::symbol<int> playerHealth(0x123456);

// Access the value
int hp = *playerHealth;
*playerHealth = 100;

// Get pointer
int* healthPtr = playerHealth.get();

// Use with structs
liborbisutil::symbol<PlayerStruct> player(0x789ABC);
player->position.x = 100.0f;
```

## Memory Functions

### Allocation

```cpp
// Allocate memory
void* buffer = liborbisutil::memory::allocate(1024);

// Free memory
liborbisutil::memory::free(buffer);
```

### Read/Write

```cpp
// Write to memory
int value = 42;
liborbisutil::memory::write(address, &value, sizeof(value));

// Read from memory
int readValue;
liborbisutil::memory::read(address, &readValue, sizeof(readValue));
```

### Memory Protection

```cpp
// Change memory protection
int oldProt;
liborbisutil::memory::protect(address, size, PROT_READ | PROT_WRITE, &oldProt);

// Query memory region
liborbisutil::memory::region info;
if (liborbisutil::memory::query(address, info)) {
    printf("Region: 0x%lx - 0x%lx, size: %zu\n", 
           info.start, info.end, info.size);
}
```

## Memory Region Structure

```cpp
struct region {
    uintptr_t start;
    uintptr_t end;
    size_t size;
    type type;      // onion, garlic_wc, garlic_wb, unknown
    int prot;       // Protection flags
};
```

## Process Base Address

```cpp
// Global process base address (initialized at startup)
extern uintptr_t process_base_address;

// Calculate absolute address
uintptr_t absolute = process_base_address + 0x1000;
```
