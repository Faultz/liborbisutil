# HTTP Module

HTTP client for making web requests on PlayStation 4.

## Initialization

```cpp
#include <liborbisutil.h>

// Initialize HTTP system
if (!liborbisutil::http::initialize()) {
    // Handle error
}

// Clean up when done
liborbisutil::http::finalize();
```

## Making Requests

### GET Request

```cpp
auto response = liborbisutil::http::get("https://api.example.com/data");

if (response.success && response.status == 200) {
    printf("Response: %s\n", response.data.c_str());
}
```

### POST Request

```cpp
std::string json_body = R"({"key": "value"})";

std::unordered_map<std::string, std::string> headers = {
    {"Content-Type", "application/json"}
};

auto response = liborbisutil::http::post(
    "https://api.example.com/submit",
    json_body,
    headers
);
```

### PUT Request

```cpp
auto response = liborbisutil::http::put(
    "https://api.example.com/update",
    update_data,
    headers
);
```

### DELETE Request

```cpp
auto response = liborbisutil::http::del("https://api.example.com/item/123");
```

### Download File

```cpp
auto response = liborbisutil::http::download(
    "https://example.com/file.zip",
    "/data/downloads/file.zip"
);
```

## Structures

### `request`

```cpp
struct request {
    std::string url;
    std::string path;
    std::string data;
    SceHttpMethods method;
    std::unordered_map<std::string, std::string> headers;
};
```

### `response`

```cpp
struct response {
    bool success;
    int req_id;
    int status;              // HTTP status code
    SceHttpMethods method;
    std::string data;        // Response body
    std::unordered_map<std::string, std::string> response_headers;
};
```

## Connection Management

For multiple requests to the same host, use explicit connections:

```cpp
auto conn = liborbisutil::http::create_connection(
    "https://api.example.com",
    "MyApp/1.0"  // User agent
);

// Make requests using connection...

// Clean up
liborbisutil::http::destroy_connection(conn);

// Or destroy all connections
liborbisutil::http::destroy_connections();
```
