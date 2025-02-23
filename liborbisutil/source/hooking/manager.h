#pragma once

#include <cstdint>
#include <unordered_map>

#include "../system/memory.h"

#include "minhook.h"

namespace liborbisutil
{
	namespace hook
	{
		class detour
		{
		public:
			detour() = default;
			detour(void* target, void* replace, bool enabled = false);
			detour(uintptr_t target, void* replace, bool enabled = false);
			~detour();

			void enable() const;
			void disable() const;

			bool create(void* target, void* replace, bool enabled = false);
			bool create(uintptr_t target, void* replace, bool enabled = false);
			bool create(const std::string& name, const std::string& module, const std::string& function, void* replace, bool enable = false);
			void clear();

			template<typename R>
			bool create(void* target, R replace, bool enabled = false)
			{
				return create(target, reinterpret_cast<void*>(replace), enabled);
			}

			template<typename R>
			bool create(uintptr_t target, R replace, bool enabled = false)
			{
				return create(target, reinterpret_cast<void*>(replace), enabled);
			}

			template<typename R>
			bool add(void* target, R replace, bool enabled = false)
			{
				return create(target, reinterpret_cast<void*>(replace), enabled);
			}

			template<typename R>
			bool add(uintptr_t target, R replace, bool enabled = false)
			{
				return create(target, reinterpret_cast<void*>(replace), enabled);
			}

			template <typename T>
			T* get() const
			{
				return static_cast<T*>(this->get_original());
			}

			template<typename R = void, typename... Args>
			R invoke(Args... args)
			{
				return reinterpret_cast<R(*)(Args...)>(this->original)(args...);
			}

			void* get_original() const;
			bool is_created() const { return this->created; }
		private:
			void* target;
			void* replace;
			void* original;

			bool created;
		};

		class manager
		{
		public:
			manager() = default;
			~manager();

			bool create(const std::string& name, void* target, void* replace, bool enable = false);
			bool create(const std::string& name, uintptr_t target, void* replace, bool enable = false);
			bool create(const std::string& name, const std::string& module, const std::string& function, void* replace, bool enable = false);

			bool enable(const std::string& name);
			bool disable(const std::string& name);

			bool remove(const std::string& name);
			void clear();

			detour* get(const std::string& name);

			template<typename R>
			bool create(const std::string& name, void* target, R replace, bool enable = false)
			{
				return create(name, target, reinterpret_cast<void*>(replace), enable);
			}

			template<typename R>
			bool create(const std::string& name, uintptr_t target, R replace, bool enable = false)
			{
				return create(name, reinterpret_cast<void*>(target), reinterpret_cast<void*>(replace), enable);
			}

			template<typename R>
			bool create(const std::string& name, const std::string& module, const std::string& function, R replace, bool enable = false)
			{
				return create(name, module, function, reinterpret_cast<void*>(replace), enable);
			}

			template<typename R>
			bool add(const std::string& name, void* target, R replace, bool enable = false)
			{
				return create(name, target, reinterpret_cast<void*>(replace), enable);
			}

			template<typename R>
			bool add(const std::string& name, uintptr_t target, R replace, bool enable = false)
			{
				return create(name, reinterpret_cast<void*>(target), reinterpret_cast<void*>(replace), enable);
			}

			template<typename R>
			bool add(const std::string& name, const std::string& module, const std::string& function, R replace, bool enable = false)
			{
				return create(name, module, function, reinterpret_cast<void*>(replace), enable);
			}

			template<typename R = void, typename... Args>
			R invoke(const std::string& name, Args... args)
			{
				return reinterpret_cast<R(*)(Args...)>(this->get(name)->get_original())(args...);
			}
		private:
			std::unordered_map<std::string, std::unique_ptr<detour>> hooks;
		};

		template <typename T>
		static void set(void* place, T value)
		{
			int prot;
			memory::protect(reinterpret_cast<uintptr_t>(place), sizeof(T), SCE_KERNEL_PROT_CPU_ALL, &prot);

			*static_cast<T*>(place) = value;

			memory::protect(reinterpret_cast<uintptr_t>(place), sizeof(T), prot, &prot);
		}
	}
}