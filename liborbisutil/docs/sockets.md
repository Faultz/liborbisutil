# Sockets Module

TCP and UDP socket wrapper for network communication.

## Usage

### Creating a TCP Socket

```cpp
#include <liborbisutil.h>

// Create socket with IP string and port
liborbisutil::socket client("192.168.1.100", 8080, liborbisutil::tcp);

// Or with numeric IP
uint32_t ip = ...; // Network byte order
liborbisutil::socket client(ip, 8080, liborbisutil::tcp);

// Or from existing file descriptor
liborbisutil::socket client(existing_fd, liborbisutil::tcp);
```

### Connecting

```cpp
if (client.connect()) {
    printf("Connected!\n");
} else {
    printf("Connection failed\n");
}
```

### Sending Data

```cpp
const char* message = "Hello, Server!";
if (client.send(message, strlen(message))) {
    printf("Data sent\n");
}

// Send binary data
struct MyPacket packet = {...};
client.send(&packet, sizeof(packet));
```

### Receiving Data

```cpp
char buffer[1024];
if (client.receive(buffer, sizeof(buffer))) {
    printf("Received: %s\n", buffer);
}

// Receive structured data
MyPacket response;
client.receive(&response, sizeof(response));
```

### Connection Status

```cpp
if (client.is_connected()) {
    // Socket is connected
}
```

### Closing

```cpp
client.close();
// Or let destructor handle it
```

## Socket Types

```cpp
enum socket_type {
    tcp,  // TCP stream socket
    udp   // UDP datagram socket
};
```

## UDP Example

```cpp
liborbisutil::socket udp_socket("192.168.1.100", 9000, liborbisutil::udp);

if (udp_socket.connect()) {
    // Send datagram
    udp_socket.send(data, data_size);
    
    // Receive datagram
    udp_socket.receive(buffer, buffer_size);
}
```

## Constants

- `INVALID_SOCKET` - Invalid socket file descriptor (-1)
- `SOCKET_TIME_OUT` - Default socket timeout (7000ms)
