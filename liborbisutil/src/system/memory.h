#pragma once

#include <memory>

uintptr_t process_base_address;

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
		bool change_protection(uintptr_t address, size_t size, int prot);

		// Template for reading data
		template <typename T>
		T read(uintptr_t address)
		{
			T value;
			read(address, &value, sizeof(T));
			return value;
		}

		// Template for writing data
		template <typename T>
		void write(uintptr_t address, const T& value)
		{
			write(address, &value, sizeof(T));
		}

		void write(uintptr_t address, const void* buffer, size_t size);
		void read(uintptr_t address, void* buffer, size_t size);
	}
}