#pragma once

// Patcher utilities - standard includes provided by stdafx.h

namespace liborbisutil
{
	namespace patcher
	{
		struct info
		{
			info(uintptr_t address)
				: address(address), set(false)
			{
			}

			bool is_set();

			void enable();
			void disable();
			void toggle();
			void restore();

			uintptr_t address;
			std::vector<uint8_t> original;
			std::vector<uint8_t> patched;
			bool set;
		};

		void enable(const std::string& name);
		void disable(const std::string& name);
		void toggle(const std::string& name);
		void restore(const std::string& name);

		info* get(const std::string& name);

		info* nop(const std::string& name, const std::string& module, uint64_t offset, size_t size);
		info* nop(const std::string& name, uintptr_t address, size_t size);

		info* create(const std::string& name, const std::string& module, uint64_t offset, const std::vector<uint8_t>& patched, const bool set = false);
		info* create(const std::string& name, uintptr_t address, const std::vector<uint8_t>& patched, const bool set = false);

		info* add(const std::string& name, const std::string& module, uint64_t offset, const std::vector<uint8_t>& patched, const bool set = false);
		info* add(const std::string& name, uintptr_t address, const std::vector<uint8_t>& patched, const bool set = false);

		void remove(const std::string& name);

		void enable_all();
		void restore_all();
	}
}