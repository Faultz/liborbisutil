#include "../../stdafx.h"
#include "patcher.h"
#include "../logger/logger.h"
#include "../resolve/resolve.h"
#include "../memory/memory.h"

namespace liborbisutil
{
	namespace patcher
	{
		namespace internal
		{
			inline std::unordered_map<std::string, std::shared_ptr<info>> patches;
		}

		bool info::is_set()
		{
			return this->set;
		}

		void info::enable()
		{
			if (this->set)
				return;

			memory::write(address, patched.data(), patched.size());
		}

		void info::disable()
		{
			if (!this->set)
				return;

			memory::write(address, original.data(), original.size());
		}

		void info::toggle()
		{
			if (this->set)
				this->disable();
			else
				this->enable();
		}

		void info::restore()
		{
			if (this->set)
				this->disable();
		}

		void enable(const std::string& name)
		{
			auto patch = get(name);
			if (patch)
				patch->enable();
		}

		void disable(const std::string& name)
		{
			auto patch = get(name);
			if (patch)
				patch->disable();
		}

		void toggle(const std::string& name)
		{
			auto patch = get(name);
			if (patch)
				patch->toggle();
		}

		void restore(const std::string& name)
		{
			auto patch = get(name);
			if (patch)
				patch->restore();
		}

		info* get(const std::string& name)
		{
			auto it = internal::patches.find(name);
			if (it != internal::patches.end())
				return it->second.get();

			return nullptr;
		}

		info* nop(const std::string& name, const std::string& module, uint64_t offset, size_t size)
		{
			auto address = resolve::get_module_address(module);
			if (!address)
			{
				LOG_ERROR("Failed to resolve export: %s\n", module.c_str());
				return nullptr;
			}

			return nop(name, address + offset, size);
		}

		info* nop(const std::string& name, uintptr_t address, size_t size)
		{
			std::vector<uint8_t> original_copy = memory::read(address, size);

			std::vector<uint8_t> nop(size, 0x90);
			auto patch = std::make_shared<info>(address);
			patch->original = original_copy;
			patch->patched = nop;
			patch->set = true;

			internal::patches[name] = patch;

			patch->enable();

			return patch.get();
		}

		info* create(const std::string& name, const std::string& module, uint64_t offset, const std::vector<uint8_t>& patched, const bool set)
		{
			auto address = resolve::get_module_address(module);
			if (!address)
			{
				LOG_ERROR("Failed to resolve export: %s\n", module.c_str());
				return nullptr;
			}

			return create(name, address + offset, patched, set);
		}

		info* create(const std::string& name, uintptr_t address, const std::vector<uint8_t>& patched, const bool set)
		{
			std::vector<uint8_t> original_copy = memory::read(address, patched.size());

			auto patch = std::make_shared<info>(address);
			patch->original = original_copy;
			patch->patched = patched;
			patch->set = set;

			internal::patches[name] = patch;

			if (set)
				patch->enable();

			return patch.get();
		}

		info* add(const std::string& name, const std::string& module, uint64_t offset, const std::vector<uint8_t>& patched, const bool set)
		{
			auto address = resolve::get_module_address(module);
			if (!address)
			{
				LOG_ERROR("Failed to resolve export: %s\n", module.c_str());
				return nullptr;
			}

			return add(name, address + offset, patched, set);
		}

		info* add(const std::string& name, uintptr_t address, const std::vector<uint8_t>& patched, const bool set)
		{
			std::vector<uint8_t> original_copy = memory::read(address, patched.size());

			auto patch = std::make_shared<info>(address);
			patch->original = original_copy;
			patch->patched = patched;
			patch->set = set;

			internal::patches[name] = patch;

			if (set)
				patch->enable();

			return patch.get();
		}

		void remove(const std::string& name)
		{
			auto it = internal::patches.find(name);
			if (it != internal::patches.end())
				internal::patches.erase(it);
		}
	}
}