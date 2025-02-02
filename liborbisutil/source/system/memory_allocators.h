#pragma once

#include <mutex>
#include <set>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include <gnmx.h>
#include <kernel.h>
#include <KernelExt.h>

#include "memory.h"

namespace liborbisutil
{
	namespace memory
	{
		class allocator
		{
		public:
			~allocator();

			void clear();
			void* allocate(const size_t length);
			void* reallocate(void* data, const size_t length);
			template<typename T>
			T* allocate(const size_t count)
			{
				return static_cast<T*>(allocate(sizeof(T) * count));
			}
			template<typename T>
			T* allocate()
			{
				return allocate<T>(sizeof(T));
			}
			template<typename T>
			void free(T* data)
			{
				free(static_cast<void*>(data));
			}
			void free(void* data);
			void free(const void* data);
			bool empty();
		private:
			std::mutex mtx;
			std::vector<void*> pointers;
		};

		class Chunk
		{
		public:
			Chunk(size_t size, void* ptr = nullptr, Chunk* prev = nullptr, Chunk* next = nullptr)
				: m_size(size), m_ptr(ptr), m_prev(prev), m_next(next) {
			}
			bool operator<(const Chunk& rhs) const
			{
				return (m_size < rhs.m_size) || (m_size == rhs.m_size
					&& (!rhs.m_ptr || reinterpret_cast<size_t>(m_ptr) < reinterpret_cast<size_t>(rhs.m_ptr)));
			}
			bool operator==(const Chunk& rhs) const
			{
				return m_ptr == rhs.m_ptr && m_size == rhs.m_size;
			}

			size_t m_size;
			void* m_ptr;
			Chunk* m_prev;
			Chunk* m_next;
		};

		class heap_allocator
		{
		public:
			heap_allocator() = default;
			heap_allocator(void* start, size_t size, const std::string& name);
			~heap_allocator();

			void set(void* start, size_t size, const std::string& name);

			void* allocate(size_t size, size_t alignment = 16, const std::string& name = "");
			void free(void* ptr);

			void clear();

			struct chunk_compare
			{
				bool operator()(const Chunk* lhs, const Chunk* rhs) const;
			};

			void* MapStart;
			size_t Size;
			size_t CurrentSize;
			std::string Name;
			std::mutex mtx;
			std::set<Chunk*, chunk_compare> free_chunks;
			std::unordered_map<void*, Chunk*> chunks;
		};

		class mapped_allocator : public heap_allocator
		{
		public:
			mapped_allocator() = default;
			mapped_allocator(size_t size, int protection = SCE_KERNEL_PROT_CPU_ALL, int flags = 0x1000 | 0x2, uintptr_t address = process_base_address);
			mapped_allocator(const mapped_allocator&) = delete;
			virtual ~mapped_allocator();

			void initialize(size_t size, int protection = SCE_KERNEL_PROT_CPU_ALL, int flags = 0x1000 | 0x2, uintptr_t address = process_base_address);

			template<typename T>
			T* allocator(const size_t count)
			{
				return static_cast<T*>(Allocate(sizeof(T) * count, 0));
			}

			void* Allocate(size_t size, const std::string& name = "");

			uintptr_t Base;
			size_t Size;
			bool Initialised;
			size_t CurrentSize;
		};

		class direct_memory_alloactor : public heap_allocator
		{
		public:
			direct_memory_alloactor() = default;
			direct_memory_alloactor(size_t size, SceKernelMemoryType type, int protection = SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW, std::string name = "");
			direct_memory_alloactor(const direct_memory_alloactor&) = delete;
			virtual ~direct_memory_alloactor();

			void initialize(size_t size, SceKernelMemoryType type, std::string name = "", int protection = SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW);

			template<typename T>
			T* Allocate(const size_t count, size_t alignment = 16)
			{
				return static_cast<T*>(Allocate(sizeof(T) * count, alignment));
			}

			void* Allocate(size_t size, size_t alignment = 16, const std::string& name = "");
			void* Allocate(const sce::Gnm::SizeAlign& sizeAlign, const std::string& name = "");

			off_t Start;
			size_t End;
			bool Initialised;
			SceKernelMemoryType Type;
		};
	}
}