#pragma once

// Memory allocators - standard includes provided by stdafx.h
// Depends on: memory

#include "../memory.h"

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
				: size(size), ptr(ptr), prev(prev), next(next) {
			}
			bool operator<(const Chunk& rhs) const
			{
				return (size < rhs.size) || (size == rhs.size
					&& (!rhs.ptr || reinterpret_cast<size_t>(ptr) < reinterpret_cast<size_t>(rhs.ptr)));
			}
			bool operator==(const Chunk& rhs) const
			{
				return ptr == rhs.ptr && size == rhs.size;
			}

			size_t size;
			void* ptr;
			Chunk* prev;
			Chunk* next;
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

			void* map_start;
			size_t cached_size;
			size_t current_size;
			std::string name;
			std::mutex mtx;
			std::set<Chunk*, chunk_compare> free_chunks;
			std::unordered_map<void*, Chunk*> chunks;
			sce::Gnm::OwnerHandle owner_handle;
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
			T* allocate(const size_t count)
			{
				return static_cast<T*>(allocate(sizeof(T) * count, 0));
			}

			void* allocate(size_t size, const std::string& name = "");

			uintptr_t base;
			bool initialised;
		};

		class direct_memory_allocator : public heap_allocator
		{
		public:
			direct_memory_allocator() = default;
			direct_memory_allocator(size_t size, SceKernelMemoryType type, std::string name = "", size_t end = SCE_KERNEL_MAIN_DMEM_SIZE, int protection = SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW);
			direct_memory_allocator(const direct_memory_allocator&) = delete;
			virtual ~direct_memory_allocator();

			void initialize(size_t size, SceKernelMemoryType type, std::string name = "", size_t end = SCE_KERNEL_MAIN_DMEM_SIZE, int protection = SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW);

			template<typename T>
			T* allocate(const size_t count, size_t alignment = 16)
			{
				return static_cast<T*>(allocate(sizeof(T) * count, alignment));
			}

			void* allocate(size_t size, size_t alignment = 16, const std::string& name = "");
			void* allocate(const sce::Gnm::SizeAlign& sizeAlign, const std::string& name = "");

			off_t start;
			bool initialised;
			SceKernelMemoryType type;
			sce::Gnm::OwnerHandle owner_handle;
		};
	}
}