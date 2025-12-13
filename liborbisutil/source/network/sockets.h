#pragma once

// Sockets - standard includes provided by stdafx.h

#define INVALID_SOCKET -1
#define SOCKET_TIME_OUT 7000

namespace liborbisutil
{
	enum socket_type
	{
		tcp,
		udp
	};

	class socket
	{
	public:
		socket() = default;
		socket(const std::string& ip, uint16_t port, socket_type type);
		socket(int fd, socket_type type);
		socket(uint32_t ip, uint16_t port, socket_type type);
		~socket() { close(); }

		bool connect();
		void close();
		bool receive(void* buffer, size_t length);
		bool send(const void* buffer, size_t length);

		bool is_connected() const { return connected; };
	private:
		int fd;
		bool connected;
		uint32_t ip;
		uint16_t port;
		socket_type type;
		SceNetSockaddrIn server_addr;
	};
}