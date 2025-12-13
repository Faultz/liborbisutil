# Filesystem Module

File I/O operations and directory traversal.

## file_system Class

Low-level file operations wrapper.

```cpp
#include <liborbisutil.h>

// Open file
liborbisutil::file_system file("/data/save.dat", "rb");
// Or with integer mode flags
liborbisutil::file_system file("/data/save.dat", O_RDONLY);

// Check if open
if (file.is_open()) {
    // File is ready
}
```

### Reading

```cpp
// Read entire file as string
std::string content = file.read_to_end();

// Read binary data
char buffer[1024];
size_t bytesRead = file.read(buffer, sizeof(buffer));

// Read at offset
size_t bytesRead = file.readOffset(buffer, sizeof(buffer), offset);

// Read line by line
std::string line;
while (file.getline(line)) {
    // Process line
}
```

### Writing

```cpp
liborbisutil::file_system file("/data/output.txt", "wb");

// Write raw data
file.write(data, size);

// Printf-style writing
file.print("Value: %d, Name: %s\n", value, name);

// Stream-style operators
file << "Hello " << 42 << " " << 3.14f;
```

### Navigation

```cpp
file.seek(100, SEEK_SET);  // Seek to position 100
file.seek(50, SEEK_CUR);   // Seek forward 50 bytes
size_t pos = file.tell();  // Get current position
size_t size = file.get_size();  // Get file size
```

## directory_iterator Class

Iterate over files in a directory.

```cpp
// Non-recursive
for (const auto& file : liborbisutil::directory_iterator("/data/saves/")) {
    printf("Found: %s\n", file.c_str());
}

// Recursive
for (const auto& file : liborbisutil::directory_iterator("/data/", true)) {
    printf("Found: %s\n", file.c_str());
}
```

## path Class

Path manipulation utilities.

```cpp
liborbisutil::path base("/data/game");

// Path concatenation
auto savePath = base / "saves" / "slot1.dat";
// Result: /data/game/saves/slot1.dat

// Get string
std::string pathStr = savePath.str();

// In-place concatenation
base /= "textures";
base /= "player.png";
```

## File Properties

```cpp
file.name;        // File name
file.fd;          // File descriptor
file.offset;      // Current offset
file.end_of_file; // EOF flag
file.mode;        // Open mode
file.length;      // File length
```
