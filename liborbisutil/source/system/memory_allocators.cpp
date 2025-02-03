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
			// If the data is null, just allocate?
			if (data == nullptr)
			{
				return allocate(length);
			}

			return nullptr;

			//std::lock_guard<std::mutex> lock(Mutex);

			//// Check if the pointers are managed by the allocator
			//auto it = std::find(Pointers.begin(), Pointers.end(), data);
			//if (it == Pointers.end()) {
			//	throw std::invalid_argument("Pointer not managed by this allocator.");
			//}

			//void* newData = Allocate(length);
			//if (!newData)
			//	throw std::bad_alloc();

			//std::memcpy(newData, data, length);

			//Free(data);

			//// Replace the old pointer with the new one in the tracking vector
			//*it = newData;

			//return newData;
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

			Size = size;
			MapStart = start;

			if (MapStart == nullptr)
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

				uintptr_t ptr = reinterpret_cast<uintptr_t>(chunk->m_ptr);

				// Calculate the offset to align the pointer if alignment != 0
				size_t offset = 0;
				if (alignment)
				{
					size_t alignedPtr = (ptr + alignment - 1) & ~(alignment - 1);
					offset = alignedPtr - ptr;
				}

				if (chunk->m_size >= (size + offset))
				{
					Chunk* current = *it;

					free_chunks.erase(it);


					size_t begPtr = ptr + offset;
					size_t endPtr = ptr + current->m_size;
					size_t newEnd = begPtr + size;

					void* newBeg = reinterpret_cast<void*>(begPtr);

					if (offset) // break off back as new chunk
					{
						auto prev = new Chunk(offset, current->m_ptr, current->m_prev, current);
						if (current->m_prev)
						{
							current->m_prev->m_next = prev;
						}
						current->m_prev = prev;
						current->m_ptr = newBeg;
						current->m_size -= offset;

						free_chunks.insert(prev);
					}

					if (newEnd != endPtr) // break this off as a new chunk
					{
						auto next = current;
						current = new Chunk(size, newBeg, next->m_prev, next);
						if (current->m_prev)
						{
							current->m_prev->m_next = current;
						}
						next->m_prev = current;
						next->m_size = endPtr - newEnd;
						next->m_ptr = reinterpret_cast<void*>(newEnd);

						free_chunks.insert(next);
					}

					chunks[current->m_ptr] = current;

					return current->m_ptr;
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

			auto cur = it->second;
			chunks.erase(it);

			if (cur->m_prev)
			{
				auto prevIt = free_chunks.find(cur->m_prev);
				if (prevIt != free_chunks.end())
				{
					auto prev = *prevIt;
					free_chunks.erase(prevIt);

					if (cur->m_next)
					{
						auto nextIt = free_chunks.find(cur->m_next);
						if (nextIt != free_chunks.end())
						{
							auto next = *nextIt;

							free_chunks.erase(nextIt);

							next->m_size += cur->m_size + prev->m_size;
							next->m_ptr = prev->m_ptr;
							next->m_prev = prev->m_prev;

							if (next->m_prev)
							{
								next->m_prev->m_next = next;
							}

							free_chunks.insert(next);

							delete cur;
							delete prev;

							return;
						}
					}

					prev->m_size += cur->m_size;
					prev->m_next = cur->m_next;

					if (prev->m_next)
					{
						prev->m_next->m_prev = prev;
					}

					free_chunks.insert(prev);

					delete cur;

					return;
				}
			}

			if (cur->m_next)
			{
				auto nextIt = free_chunks.find(cur->m_next);
				if (nextIt != free_chunks.end())
				{
					auto next = *nextIt;

					free_chunks.erase(nextIt);

					next->m_size += cur->m_size;
					next->m_prev = cur->m_prev;
					next->m_ptr = cur->m_ptr;

					if (next->m_prev)
					{
						next->m_prev->m_next = next;
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

			memset(MapStart, 0, Size);

			auto res = sceKernelMunmap(MapStart, Size);
			if (res < 0)
			{
				LOG_ERROR("[mapped_allocator]: Failed to unmap memory: 0x%X\n", res);
				return;
			}

#ifdef _DEBUG
			LOG_DEBUG("[mapped_allocator]: Unmapped memory at 0x%lX\n", reinterpret_cast<uintptr_t>(MapStart));
#endif

			Initialised = false;
		}

		void mapped_allocator::initialize(size_t size, int protection, int flags, uintptr_t address)
		{
			Size = size;
			CurrentSize = size;
			Base = address;

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

			Initialised = true;
		}

		void* mapped_allocator::allocate(size_t size, const std::string& name)
		{
			if (!Initialised)
			{
				LOG_ERROR("[mapped_allocator]: Not initialised\n");
				return nullptr;
			}

			auto ptr = heap_allocator::allocate(size, 0, name);
			if (ptr)
			{
				CurrentSize -= size;
				if (CurrentSize < 0)
				{
					LOG_ERROR("[mapped_allocator]: Overallocated memory\n");
				}

				return ptr;
			}

			return nullptr;
		}

		direct_memory_allocator::direct_memory_allocator(size_t size, SceKernelMemoryType type, int protection, std::string name)
		{
			initialize(size, type, name, protection);
		}

		direct_memory_allocator::~direct_memory_allocator()
		{
			clear();

			memset(MapStart, 0, Size);

			auto res = sceKernelMunmap(MapStart, Size);
			if (res < 0)
			{
				LOG_ERROR("Failed to unmap memory: 0x%X\n", res);
			}

			LOG_DEBUG("[direct_memory_allocator]: Unmapped memory at 0x%08lX\n", reinterpret_cast<uintptr_t>(MapStart));

			res = sceKernelReleaseDirectMemory(Start, Size);
			if (res < 0)
			{
				LOG_ERROR("[direct_memory_allocator]: Failed to release memory: 0x%X\n", res);
			}

			LOG_DEBUG("[direct_memory_allocator]: Released memory at 0x%08lX\n", Start);

			Initialised = false;
		}

		void direct_memory_allocator::initialize(size_t size, SceKernelMemoryType type, std::string name, int protection)
		{
			Name = name;
			Size = size;
			Type = type;
			Size = CurrentSize = size;
			auto res = sceKernelAllocateDirectMemory(0, SCE_KERNEL_MAIN_DMEM_SIZE + 50 * 1024 * 1024, size, 0, type, &Start);
			if (res < 0)
			{
				LOG_ERROR("[direct_memory_allocator]: Failed to allocate memory: 0x%X\n", res);
				return;
			}

			void* mapStart = nullptr;
			res = sceKernelMapNamedDirectMemory(&mapStart, size, protection, 0, Start, 0, name.data());
			if (res < 0)
			{
				LOG_ERROR("[direct_memory_allocator]: Failed to map memory: 0x%X\n", res);
				return;
			}

			memset(mapStart, 0, size);

			set(mapStart, size, name);

			Initialised = true;
		}

		void* direct_memory_allocator::allocate(size_t size, size_t alignment, const std::string& name)
		{
			if (!Initialised)
			{
				LOG_ERROR("[direct_memory_allocator]: Not initialised\n");
				return nullptr;
			}

			auto ptr = heap_allocator::allocate(size, alignment, name);
			if (ptr)
			{
				CurrentSize -= size;
				if (CurrentSize < 0)
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