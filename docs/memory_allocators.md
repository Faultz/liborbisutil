# Memory Allocators Module

Custom memory allocators for managed allocation tracking and heap management.

## Allocator Class

Simple allocator that tracks all allocations for bulk cleanup.

```cpp
#include <liborbisutil.h>

liborbisutil::memory::allocator alloc;

// Allocate raw memory
void* buffer = alloc.allocate(1024);

// Allocate typed memory
int* intArray = alloc.allocate<int>(100);  // 100 ints
MyStruct* obj = alloc.allocate<MyStruct>();

// Reallocate
buffer = alloc.reallocate(buffer, 2048);

// Free individual allocation
alloc.free(buffer);

// Free all allocations at once
alloc.clear();

// Check if empty
if (alloc.empty()) {
    // No active allocations
}
```

## Heap Allocator Class

Fixed-size heap allocator with chunk management for efficient allocation/deallocation.

```cpp
// Create allocator with pre-allocated buffer
void* heapMemory = /* your large buffer */;
size_t heapSize = 64 * 1024 * 1024;  // 64 MB

liborbisutil::memory::heap_allocator heap(heapMemory, heapSize, "GameHeap");

// Or set later
liborbisutil::memory::heap_allocator heap;
heap.set(heapMemory, heapSize, "GameHeap");

// Allocate with alignment
void* data = heap.allocate(1024, 16, "PlayerData");
void* textures = heap.allocate(4096, 256, "TextureBuffer");

// Free memory
heap.free(data);

// Clear entire heap
heap.clear();
```

## Heap Allocator Features

- **Chunk-based allocation** - Efficient memory reuse
- **Alignment support** - Specify alignment requirements
- **Named allocations** - Debug-friendly allocation names
- **Thread-safe** - Mutex-protected operations
- **Memory tracking** - Track current vs. cached size

## Heap Allocator Properties

```cpp
heap.map_start;      // Start of heap memory
heap.cached_size;    // Total heap size
heap.current_size;   // Currently allocated size
heap.name;           // Heap name for debugging
```

## Use Cases

### Game Object Pool

```cpp
liborbisutil::memory::heap_allocator entityHeap(buffer, size, "Entities");

Entity* entity = static_cast<Entity*>(
    entityHeap.allocate(sizeof(Entity), alignof(Entity), "Entity")
);

// ... use entity ...

entityHeap.free(entity);
```

### Frame Allocator

```cpp
liborbisutil::memory::allocator frameAlloc;

void RenderFrame() {
    // Allocate temporary data
    auto* vertices = frameAlloc.allocate<Vertex>(1000);
    
    // ... render ...
    
    // Clear at end of frame
    frameAlloc.clear();
}
```
