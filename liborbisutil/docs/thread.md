# Thread Module

Threading utilities, thread management, and thread pool.

## thread Class

POSIX thread wrapper for PS4.

```cpp
#include <liborbisutil.h>

// Thread function
void my_thread_func(void* arg) {
    int* value = static_cast<int*>(arg);
    while (true) {
        // Do work
        liborbisutil::thread::sleep(100);  // Sleep 100ms
    }
}

// Create and start thread
int data = 42;
liborbisutil::thread worker(my_thread_func, "WorkerThread", &data, 0, false);

// Check state
if (worker.is_running()) {
    // Thread is active
}

// Wait for thread to complete
if (worker.joinable()) {
    worker.join();
}

// Or detach
worker.detach();

// Stop thread
worker.stop();
```

## Threading Namespace

System-wide thread management.

```cpp
// Get current thread
posix_thread* current = liborbisutil::threading::get_current();

// Find threads
posix_thread* thread = liborbisutil::threading::get_by_id(threadId);
posix_thread* thread = liborbisutil::threading::get_by_name("WorkerThread");

// Get thread name
std::string name = liborbisutil::threading::get_name(thread);

// List all threads
std::vector<posix_thread*> threads = liborbisutil::threading::get_threads();
size_t count = liborbisutil::threading::count();
```

### Thread Control

```cpp
// Freeze/unfreeze threads (useful for debugging or memory operations)
liborbisutil::threading::freeze(thread);
liborbisutil::threading::unfreeze(thread);

// Freeze all threads except current
auto frozen = liborbisutil::threading::freeze(true, false);
// ... do critical work ...
liborbisutil::threading::unfreeze(frozen);
```

### Thread Creation

```cpp
// Create with more control
ScePthread thread = liborbisutil::threading::create(
    "MyThread",
    [](void* arg) -> void* {
        // Thread work
        return nullptr;
    },
    true,  // joinable
    nullptr  // arg
);

liborbisutil::threading::join(thread);
// Or
liborbisutil::threading::detach(thread);
```

### Sleep

```cpp
liborbisutil::threading::sleep(1000);  // Sleep 1 second
```

## Thread Pool

Efficient task execution across multiple worker threads.

```cpp
#include <liborbisutil.h>

// Push tasks and get futures
auto future1 = liborbisutil::thread_pool::push([]() {
    return expensive_calculation();
});

auto future2 = liborbisutil::thread_pool::push([](int x) {
    return x * 2;
}, 21);

// Wait for results
int result1 = future1.get();
int result2 = future2.get();  // 42

// Push tasks with arguments
auto future = liborbisutil::thread_pool::push(process_data, data, size);
```

## Use Cases

### Background Loading

```cpp
auto loadFuture = liborbisutil::thread_pool::push([path]() {
    return load_texture(path);
});

// Continue main thread work...

// Get loaded texture when needed
Texture* tex = loadFuture.get();
```

### Parallel Processing

```cpp
std::vector<std::future<int>> futures;

for (int i = 0; i < 10; i++) {
    futures.push_back(liborbisutil::thread_pool::push(process_chunk, i));
}

// Collect results
for (auto& f : futures) {
    results.push_back(f.get());
}
```
