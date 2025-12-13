#include "../../stdafx.h"
#include "../logger/logger.h"
#include "memory.h"

namespace liborbisutil
{
	namespace memory
	{
		uintptr_t read_offset(uintptr_t address)
		{
			if (address == 0) {
				LOG_ERROR("Attempted to read a null offset!");
				return 0;
			}

			if constexpr (sizeof(uintptr_t) == sizeof(uint64_t)) {
				return address + *reinterpret_cast<int32_t*>(address) + 4;
			}
			else {
				return *reinterpret_cast<uintptr_t*>(address);
			}
		}

		void* allocate(size_t size)
		{
			return malloc(size);
		}

		void free(void* ptr)
		{
			if (ptr)
			{
				free(ptr);
			}
		}

		bool query(uintptr_t address, region& out)
		{
			memset(&out, 0, sizeof(region));

			SceKernelVirtualQueryInfo info;
			auto res = sceKernelVirtualQuery((void*)address, 0, &info, sizeof(SceKernelVirtualQueryInfo));
			if (res < 0)
			{
				return false;
			}

			auto convert_memory_type = [](int t) -> memory::type
				{
					switch (t)
					{
					case SCE_KERNEL_WB_ONION:
						return onion;
					case SCE_KERNEL_WC_GARLIC:
						return garlic_wc;
					case SCE_KERNEL_WB_GARLIC:
						return garlic_wb;
					default:
						return unknown;
					}
				};

			if (!info.isCommitted && info.protection == PROT_NONE)
			{
#ifdef _DEBUG
				printf("Memory is not committed\n");
#endif
				return false;
			}

			out.start = reinterpret_cast<uintptr_t>(info.start);
			out.end = reinterpret_cast<uintptr_t>(info.end);
			out.size = reinterpret_cast<uintptr_t>(info.end) - reinterpret_cast<uintptr_t>(info.start);
			out.type = convert_memory_type(info.memoryType);
			out.prot = info.protection;

			return true;
		}

		bool protect(uintptr_t address, size_t size, int prot, int* prev)
		{
			if (prot)
			{
				auto res = sceKernelQueryMemoryProtection(reinterpret_cast<void*>(address), nullptr, nullptr, prev);
				if (res < 0)
				{
					printf("sceKernelQueryMemoryProtection(0x%lX) failed: 0x%X\n", address, res);
					return false;
				}
			}

			auto res = sceKernelMprotect(reinterpret_cast<void*>(address), size, prot);
			if (res < 0)
			{
				return false;
			}

			return true;
		}

		void write(uintptr_t address, const void* buffer, size_t size)
		{
			int prev;
			protect(address, size, SCE_KERNEL_PROT_CPU_ALL, &prev);
			__builtin_memcpy(reinterpret_cast<void*>(address), buffer, size);
			protect(address, size, prev);
		}

		void read(uintptr_t address, void* buffer, size_t size)
		{
			int prev;
			protect(address, size, SCE_KERNEL_PROT_CPU_ALL, &prev);
			__builtin_memcpy(buffer, reinterpret_cast<void*>(address), size);
			protect(address, size, prev);
		}
	}
}