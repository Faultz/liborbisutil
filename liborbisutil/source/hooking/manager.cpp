#include "manager.h"
#include "../system/logger.h"
#include "../system/resolve.h"

namespace liborbisutil
{
	namespace hook
	{
		detour::detour(void* target, void* replace, bool enabled)
		{
			this->original = nullptr;
			this->create(target, replace, enabled);
		}

		detour::detour(uintptr_t target, void* replace, bool enabled)
		{
			this->original = nullptr;
			this->create(target, replace, enabled);
		}

		detour::~detour()
		{
			this->clear();
		}

		void detour::enable() const
		{
			if (this->target)
			{
				if (MH_EnableHook(this->target) != MH_OK)
				{
					LOG_ERROR("Failed to enable hook\n");
				}
			}
		}

		void detour::disable() const
		{
			if (this->target)
			{
				if (MH_DisableHook(this->target) != MH_OK)
				{
					LOG_ERROR("Failed to disable hook\n");
				}
			}
		}

		bool detour::create(void* target, void* replace, bool enabled)
		{
			this->target = target;
			this->replace = replace;

			void* original_function = nullptr;
			MH_STATUS status = MH_CreateHook(target, replace, &original_function);
			if (status != MH_OK)
			{
				LOG_ERROR("Failed to create hook: %s\n", MH_StatusToString(status));
				return false;
			}

			LOG_SUCCESS("Hooked function at 0x%08llX\n", target);

			this->created = true;
			this->original = original_function;

			if(enabled)
				this->enable();

			return true;
		}

		bool detour::create(uintptr_t target, void* replace, bool enabled)
		{
			return this->create(reinterpret_cast<void*>(target), replace, enabled);
		}

		bool detour::create(const std::string& name, const std::string& module, const std::string& function, void* replace, bool enable)
		{
			auto address = resolve::get_export_address(module, function);
			if (!address)
			{
				LOG_ERROR("Failed to resolve export: %s\n", function.c_str());
				return false;
			}
			LOG_SUCCESS("Resolved export %s at 0x%08llX\n", function.c_str(), address);
			return create(reinterpret_cast<void*>(address), replace);
		}

		void detour::clear()
		{
			if (this->target)
				MH_RemoveHook(this->target);
		}

		void* detour::get_original() const
		{
			return original;
		}

		manager::~manager()
		{
			LOG_INFO("Unhooking all functions\n");

			this->clear();
		}

		bool manager::create(const std::string& name, void* target, void* replace, bool enable)
		{
			hooks[name] = std::make_unique<detour>();
			auto hook = hooks[name].get();
			hook->create(target, replace, false);
			if (hook->is_created())
			{
				if (enable)
				{
					hook->enable();
				}

				return true;
			}
			return false;
		}

		bool manager::create(const std::string& name, uintptr_t target, void* replace, bool enable)
		{
			return create(name, reinterpret_cast<void*>(target), replace, enable);
		}

		bool manager::create(const std::string& name, const std::string& module, const std::string& function, void* replace, bool enable)
		{
			auto address = resolve::get_export_address(module, function);
			if (!address)
			{
				LOG_ERROR("Failed to resolve export: %s\n", function.c_str());
				return false;
			}

			LOG_SUCCESS("Resolved export %s at 0x%08llX\n", function.c_str(), address);

			return create(name, reinterpret_cast<void*>(address), replace, enable);
		}

		bool manager::enable(const std::string& name)
		{
			auto hook = get(name);
			if (hook)
			{
				hook->enable();
				return true;
			}

			return false;
		}

		bool manager::disable(const std::string& name)
		{
			auto hook = get(name);
			if (hook)
			{
				hook->disable();
				return true;
			}

			return false;
		}

		bool manager::remove(const std::string& name)
		{
			auto hook = hooks.find(name);
			if (hook != hooks.end())
			{
				hook->second->clear();
				hooks.erase(hook);
				return true;
			}
			return false;
		}

		void manager::clear()
		{
			for (auto& hook : hooks)
			{
				hook.second->clear();
			}

			hooks.clear();
		}
		detour* manager::get(const std::string& name)
		{
			auto hook = hooks.find(name);
			if (hook != hooks.end())
			{
				return hook->second.get();
			}
			return nullptr;
		}
	}
}