#pragma once

#include <memory>
#include <vector>

#include <KernelExt.h>

inline static uintptr_t process_base_address = []() -> uintptr_t {
	if (process_base_address)
		return process_base_address;

	SceKernelModuleInfo processModuleInfo;
	processModuleInfo.size = sizeof(SceKernelModuleInfo);
	if (sceKernelGetModuleInfo(0, &processModuleInfo) < 0)
	{
		printf("Failed to get process module info\n");
		return 0;
	}

	SceAppInfo appInfo;
	sceKernelGetAppInfo(getpid(), &appInfo);

	printf("loaded liborbisutil on %s\n", appInfo.TitleId);

	return reinterpret_cast<uintptr_t>(processModuleInfo.segmentInfo->address);
	}();


namespace liborbisutil
{
	template<typename T>
	class symbol
	{
	public:
		symbol(uintptr_t address, uintptr_t base = process_base_address) : address(address), base(base) {}

		T* get()
		{
			return reinterpret_cast<T*>(base + address);
		}

		constexpr operator T* () const
		{
			return this->get();
		}

		constexpr T* operator->() const
		{
			return get();
		}

		constexpr symbol<T>& operator=(uintptr_t address)
		{
			this->address = address;
			return *this;
		}

		constexpr operator uintptr_t() const
		{
			return reinterpret_cast<uintptr_t>(this->get());
		}

		uintptr_t get_address() const { return address; }
		uintptr_t get_base() const { return base; }
	private:
		uintptr_t address;
		uintptr_t base;
	};

	namespace memory
	{
		enum type
		{
			onion,
			garlic_wc,
			garlic_wb,
			unknown
		};

		struct region 
		{
			uintptr_t start;
			uintptr_t end;
			size_t size;
			type type;
			int prot;
		};

		void* allocate(size_t size);
		void free(void* ptr);

		bool query(uintptr_t address, region& out);
		bool protect(uintptr_t address, size_t size, int prot, int* prev = nullptr);

		void write(uintptr_t address, const void* buffer, size_t size);
		void read(uintptr_t address, void* buffer, size_t size);

		// Template for reading data
		template <typename T>
		T read(uintptr_t address)
		{
			T value;
			read(address, reinterpret_cast<void*>(&value), sizeof(T));
			return value;
		}

		template<class T = uint8_t>
		std::vector<T> read(uintptr_t address, size_t size)
		{
			std::vector<T> buffer(size);
			read(address, buffer.data(), size);
			return buffer;
		}

		// Template for writing data
		template <class T>
		void write(uintptr_t address, T* value, size_t count = 0)
		{
			write(address, reinterpret_cast<const void*>(value), sizeof(T) * count);
		}

		template<class T = uint8_t>
		void write(uintptr_t address, const std::vector<T>& buffer)
		{
			write(address, (T*)buffer.data(), buffer.size());
		}
	}
}