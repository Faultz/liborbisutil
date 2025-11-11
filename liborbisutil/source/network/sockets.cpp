#include "sockets.h"
#include "../system/resolve.h"
#include "../system/logger.h"

namespace liborbisutil
{
	namespace
	{
		uint64_t get_tick_count()
		{
			SceRtcTick time;
			sceRtcGetCurrentTick(&time);
			return time.tick;
		}

		bool resolve_ntoa(const std::string& hostname, SceNetInAddr* addr)
		{
			auto res = sceNetPoolCreate("resolver pool", 4_KB, 0);
			if (res < 0)
			{
				printf("Failed to create pool: 0x%08X\n", res);
				return false;
			}

			auto memory_id = res;

			res = sceNetResolverCreate("net resolver", memory_id, 0);
			if (res < 0)
			{
				printf("Failed to create resolver: 0x%08X\n", res);
				sceNetPoolDestroy(memory_id);
				return false;
			}

			auto net_resolver_id = res;

			res = sceNetResolverStartNtoa(net_resolver_id, hostname.c_str(), addr, 0, 0, 0);
			if (res < 0)
			{
				printf("Failed to resolve hostname: 0x%08X\n", res);
				sceNetResolverDestroy(net_resolver_id);
				sceNetPoolDestroy(memory_id);
				return false;
			}

			sceNetResolverDestroy(net_resolver_id);
			sceNetPoolDestroy(memory_id);

			return true;
		}
	}

	socket::socket(const std::string& ip, uint16_t port, socket_type type)
		: port(port), type(type)
	{
		server_addr.sin_len = sizeof(server_addr);
		server_addr.sin_port = sceNetHtons(port);
		server_addr.sin_family = SCE_NET_AF_INET;
		if (sceNetInetPton(SCE_NET_AF_INET, ip.c_str(), &server_addr.sin_addr) < 0)
		{
			printf("Failed to resolve IP address: %s\n", ip.c_str());
			return;
		}

		if (!resolve_ntoa(ip, &server_addr.sin_addr))
		{
			printf("Failed to resolve IP address: %s\n", ip.c_str());
			return;
		}

		this->connected = false;
		this->ip = server_addr.sin_addr.s_addr;
		this->fd = INVALID_SOCKET;
	}

	socket::socket(int fd, socket_type type)
	{
		this->fd = fd;
		this->type = type;
		this->connected = (fd != INVALID_SOCKET);
	}

	socket::socket(uint32_t ip, uint16_t port, socket_type type)
		: ip(ip), port(port), type(type)
	{

		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_len = sizeof(server_addr);
		server_addr.sin_family = SCE_NET_AF_INET;
		server_addr.sin_addr.s_addr = ip;
		server_addr.sin_port = sceNetHtons(port);

		this->connected = false;
		this->fd = INVALID_SOCKET;
	}



	bool socket::connect()
	{
		if (fd != INVALID_SOCKET)
		{
			LOG_ERROR("Socket is already connected\n");
			return false;
		}
		fd = sceNetSocket("liborbisutil", SCE_NET_AF_INET, type == tcp ? SCE_NET_SOCK_STREAM : SCE_NET_SOCK_DGRAM, type == tcp ? SCE_NET_IPPROTO_TCP : 0);
		if (fd < 0)
		{
			LOG_ERROR("Failed to create socket: 0x%08X\n", fd);
			return false;
		}

		if (type == tcp)
		{
			auto res = sceNetConnect(fd, reinterpret_cast<SceNetSockaddr*>(&server_addr), sizeof(server_addr));
			if (res < 0)
			{
				LOG_ERROR("Failed to connect to server: 0x%08X\n", res);
				sceNetSocketClose(fd);
				fd = INVALID_SOCKET;
				return false;
			}
		}

		connected = true;
		return true;
	}

	void socket::close()
	{
		if (fd != INVALID_SOCKET)
		{
			sceNetSocketClose(fd);
			fd = INVALID_SOCKET;
		}
	}

	bool socket::receive(void* buffer, size_t length)
	{
		if (fd == INVALID_SOCKET)
		{
			LOG_ERROR("Socket is not connected\n");
			return false;
		}

		char* buf = reinterpret_cast<char*>(buffer);
		auto start_time = get_tick_count();
		size_t data_remaining = length;

		int recv_length = 0;
		while (data_remaining > 0)
		{
			if ((get_tick_count() - start_time) > SOCKET_TIME_OUT)
			{
				LOG_ERROR("Failed to receive data: Timeout\n");
				return false;
			}

			int chunk = std::min<int>(2048, data_remaining);
			if (type == tcp)
			{
				recv_length = sceNetRecv(fd, buf, chunk, 0);
			}
			else
			{
				SceNetSocklen_t len;
				recv_length = sceNetRecvfrom(fd, buf, chunk, 0, reinterpret_cast<SceNetSockaddr*>(&server_addr), &len);
			}

			if (recv_length < 0)
			{
				LOG_ERROR("Failed to receive data: 0x%08X\n", recv_length);
				return false;
			}
			else if (recv_length < chunk)
			{
				return true;
			}

			data_remaining -= recv_length;
			buf += recv_length;
		}

		return true;
	}

	bool socket::send(const void* buffer, size_t length)
	{
		if (fd == INVALID_SOCKET)
		{
			LOG_ERROR("Socket is not connected\n");
			return false;
		}
		const char* buf = reinterpret_cast<const char*>(buffer);
		auto start_time = get_tick_count();
		size_t data_remaining = length;
		int sent_length = 0;
		while (data_remaining > 0)
		{
			if ((get_tick_count() - start_time) > SOCKET_TIME_OUT)
			{
				LOG_ERROR("Failed to send data: Timeout\n");
				return false;
			}
			int chunk = std::min<int>(2048, data_remaining);
			if (type == tcp)
			{
				sent_length = sceNetSend(fd, buf, chunk, 0);
			}
			else
			{
				sent_length = sceNetSendto(fd, buf, chunk, 0, reinterpret_cast<SceNetSockaddr*>(&server_addr), sizeof(server_addr));
			}
			if (sent_length < 0)
			{
				LOG_ERROR("Failed to send data: 0x%08X\n", sent_length);
				return false;
			}
			data_remaining -= sent_length;
			buf += sent_length;
		}
		return true;
	}
}