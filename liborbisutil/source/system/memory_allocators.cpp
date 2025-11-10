#include "memory_allocators.h"

#include "logger.h"

namespace liborbisutil
{
	namespace memory
	{
		allocator::~allocator()
		{
			clear();
		}

		void allocator::clear()
		{
			std::lock_guard<std::mutex> lock(mtx);

			for (auto ptr : pointers)
				free(ptr);
			pointers.clear();
		}

		void* allocator::allocate(const size_t length)
		{
			std::lock_guard<std::mutex> lock(mtx);

			auto data = memory::allocate(length);
			if (data)
				pointers.push_back(data);
			return data;
		}

		void* allocator::reallocate(void* data, const size_t length)
		{
			return nullptr;
		}

		void allocator::free(void* data)
		{
			std::lock_guard<std::mutex> lock(mtx);

			const auto it = std::find(pointers.begin(), pointers.end(), data);
			if (it != pointers.end())
			{
				memory::free(data);

				pointers.erase(it);
			}
		}

		void allocator::free(const void* data)
		{
			free(const_cast<void*>(data));
		}

		bool allocator::empty()
		{
			return pointers.empty();
		}

		heap_allocator::heap_allocator(void* start, size_t size, const std::string& name)
		{
			set(start, size, name);
		}

		heap_allocator::~heap_allocator()
		{
			clear();
		}

		void heap_allocator::set(void* start, size_t size, const std::string& name)
		{
			for (auto& chunk : chunks)
			{
				delete chunk.second;
			}

			for (auto& free : free_chunks)
			{
				delete free;
			}

			cached_size = size;
			map_start = start;

			if (map_start == nullptr)
			{
				printf("HeapAllocator: Start is null\n");
				return;
			}

			if (size <= 0)
			{
				printf("HeapAllocator: Size is 0\n");
				return;
			}

			free_chunks.insert(new Chunk(size, start));
		}

		void* heap_allocator::allocate(size_t size, size_t alignment, const std::string& name)
		{
			std::unique_lock<std::mutex> lock(mtx);

			// Find a chunk that fits the size
			Chunk c(size);
			auto it = free_chunks.lower_bound(&c);
			for (; it != free_chunks.end(); ++it)
			{
				auto chunk = *it;

				uintptr_t ptr = reinterpret_cast<uintptr_t>(chunk->ptr);

				// Calculate the offset to align the pointer if alignment != 0
				size_t offset = 0;
				if (alignment)
				{
					size_t alignedPtr = (ptr + alignment - 1) & ~(alignment - 1);
					offset = alignedPtr - ptr;
				}

				if (chunk->size >= (size + offset))
				{
					Chunk* current = *it;

					free_chunks.erase(it);


					size_t begPtr = ptr + offset;
					size_t endPtr = ptr + current->size;
					size_t newEnd = begPtr + size;

					void* newBeg = reinterpret_cast<void*>(begPtr);

					if (offset) // break off back as new chunk
					{
						auto prev = new Chunk(offset, current->ptr, current->prev, current);
						if (current->prev)
						{
							current->prev->next = prev;
						}
						current->prev = prev;
						current->ptr = newBeg;
						current->size -= offset;

						free_chunks.insert(prev);
					}

					if (newEnd != endPtr) // break this off as a new chunk
					{
						auto next = current;
						current = new Chunk(size, newBeg, next->prev, next);
						if (current->prev)
						{
							current->prev->next = current;
						}
						next->prev = current;
						next->size = endPtr - newEnd;
						next->ptr = reinterpret_cast<void*>(newEnd);

						free_chunks.insert(next);
					}

					chunks[current->ptr] = current;

					return current->ptr;
				}
			}
			printf("HeapAllocator: Failed to allocate memory\n");
			return nullptr;
		}

		void heap_allocator::free(void* ptr)
		{
			if (!ptr)
			{
				return;
			}

			std::unique_lock<std::mutex> l(mtx);

			auto it = chunks.find(ptr);
			if (it == chunks.end())
			{
				return;
			}

			auto cur = it->second;
			chunks.erase(it);

			if (cur->prev)
			{
				auto prevIt = free_chunks.find(cur->prev);
				if (prevIt != free_chunks.end())
				{
					auto prev = *prevIt;
					free_chunks.erase(prevIt);

					if (cur->next)
					{
						auto nextIt = free_chunks.find(cur->next);
						if (nextIt != free_chunks.end())
						{
							auto next = *nextIt;

							free_chunks.erase(nextIt);

							next->size += cur->size + prev->size;
							next->ptr = prev->ptr;
							next->prev = prev->prev;

							if (next->prev)
							{
								next->prev->next = next;
							}

							free_chunks.insert(next);

							delete cur;
							delete prev;

							return;
						}
					}

					prev->size += cur->size;
					prev->next = cur->next;

					if (prev->next)
					{
						prev->next->prev = prev;
					}

					free_chunks.insert(prev);

					delete cur;

					return;
				}
			}

			if (cur->next)
			{
				auto nextIt = free_chunks.find(cur->next);
				if (nextIt != free_chunks.end())
				{
					auto next = *nextIt;

					free_chunks.erase(nextIt);

					next->size += cur->size;
					next->prev = cur->prev;
					next->ptr = cur->ptr;

					if (next->prev)
					{
						next->prev->next = next;
					}

					free_chunks.insert(next);

					delete cur;

					return;
				}
			}

			free_chunks.insert(cur);
		}

		void heap_allocator::clear()
		{
			std::lock_guard<std::mutex> lock(mtx);

			for (auto& chunk : chunks)
			{
				delete chunk.second;
			}

			for (auto& free : free_chunks)
			{
				delete free;
			}

			free_chunks.clear();
			chunks.clear();
		}

		bool heap_allocator::chunk_compare::operator() (const Chunk* lhs, const Chunk* rhs) const
		{
			return (*lhs) < (*rhs);
		}

		mapped_allocator::mapped_allocator(size_t size, int protection, int flags, uintptr_t address)
		{
			initialize(size, protection, flags, address);
		}

		mapped_allocator::~mapped_allocator()
		{
			clear();

			memset(map_start, 0, Size);

			auto res = sceKernelMunmap(map_start, Size);
			if (res < 0)
			{
				LOG_ERROR("[mapped_allocator]: Failed to unmap memory: 0x%X\n", res);
				return;
			}

#ifdef _DEBUG
			LOG_DEBUG("[mapped_allocator]: Unmapped memory at 0x%lX\n", reinterpret_cast<uintptr_t>(map_start));
#endif

			initialised = false;
		}

		void mapped_allocator::initialize(size_t size, int protection, int flags, uintptr_t address)
		{
			cached_size = size;
			current_size = size;
			base = address;

			if (size <= 0)
			{
				LOG_ERROR("[mapped_allocator]: Size is 0\n");
				return;
			}

			void* mappedAddress = nullptr;
			auto res = sceKernelMmap(reinterpret_cast<void*>(address), size, protection, flags, -1, 0, &mappedAddress);
			if (res < 0)
			{
				LOG_ERROR("[mapped_allocator]: Failed to map memory: 0x%X\n", res);
				return;
			}

			if (mappedAddress == nullptr)
			{
				LOG_ERROR("[mapped_allocator]: Address is null\n");
				return;
			}

			set(mappedAddress, size, "[mapped_allocator]");

			initialised = true;
		}

		void* mapped_allocator::allocate(size_t size, const std::string& name)
		{
			if (!initialised)
			{
				LOG_ERROR("[mapped_allocator]: Not initialised\n");
				return nullptr;
			}

			auto ptr = heap_allocator::allocate(size, 0, name);
			if (ptr)
			{
				current_size -= size;
				if (current_size < 0)
				{
					LOG_ERROR("[mapped_allocator]: Overallocated memory\n");
				}

				return ptr;
			}

			return nullptr;
		}

		direct_memory_allocator::direct_memory_allocator(size_t size, SceKernelMemoryType type, std::string name, size_t end, int protection)
		{
			initialize(size, type, name, end, protection);
		}

		direct_memory_allocator::~direct_memory_allocator()
		{
			clear();

			memset(map_start, 0, cached_size);

			auto res = sceKernelMunmap(map_start, cached_size);
			if (res < 0)
			{
				LOG_ERROR("Failed to unmap memory: 0x%X\n", res);
			}

			LOG_DEBUG("[direct_memory_allocator]: Unmapped memory at 0x%08lX\n", reinterpret_cast<uintptr_t>(map_start));

			res = sceKernelReleaseDirectMemory(start, cached_size);
			if (res < 0)
			{
				LOG_ERROR("[direct_memory_allocator]: Failed to release memory: 0x%X\n", res);
			}

			LOG_DEBUG("[direct_memory_allocator]: Released memory at 0x%08lX\n", start);

			initialised = false;
		}

		void direct_memory_allocator::initialize(size_t size, SceKernelMemoryType type, std::string name, size_t end, int protection)
		{
			name = name;
			cached_size = size;
			current_size = size;
			type = type;
			auto res = sceKernelAllocateDirectMemory(0, SCE_KERNEL_MAIN_DMEM_SIZE + 50 * 1024 * 1024, size, 0, type, &start);
			if (res < 0)
			{
				LOG_ERROR("[direct_memory_allocator]: Failed to allocate memory: 0x%X\n", res);
				return;
			}

			void* mapStart = nullptr;
			res = sceKernelMapNamedDirectMemory(&mapStart, size, protection, 0, start, 0, name.data());
			if (res < 0)
			{
				LOG_ERROR("[direct_memory_allocator]: Failed to map memory: 0x%X\n", res);
				return;
			}

			memset(mapStart, 0, size);

			set(mapStart, size, name);

			initialised = true;
		}

		void* direct_memory_allocator::allocate(size_t size, size_t alignment, const std::string& name)
		{
			if (!initialised)
			{
				LOG_ERROR("[direct_memory_allocator]: Not initialised\n");
				return nullptr;
			}

			auto ptr = heap_allocator::allocate(size, alignment, name);
			if (ptr)
			{
				current_size -= size;
				if (current_size < 0)
				{
					LOG_ERROR("[direct_memory_allocator]: Overallocated memory\n");
				}

				return ptr;
			}

			return nullptr;
		}

		void* direct_memory_allocator::allocate(const sce::Gnm::SizeAlign& sizeAlign, const std::string& name)
		{
			return allocate(sizeAlign.m_size, sizeAlign.m_align, name);
		}
	}
}