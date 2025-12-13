#pragma once

// Resolve utilities - standard includes provided by stdafx.h

constexpr size_t operator"" _KB(unsigned long long size)
{
	return static_cast<size_t>(size * 1024);
}

constexpr size_t operator"" _MB(unsigned long long size)
{
	return static_cast<size_t>(size * 1024_KB);
}

constexpr size_t operator"" _GB(unsigned long long size)
{
	return static_cast<size_t>(size * 1024_MB);
}

namespace liborbisutil
{
	namespace resolve
	{
		uintptr_t get_process_base();
		uintptr_t get_export_address(const std::string& module, const std::string& function);

		size_t get_module_size(const std::string& module);
		SceKernelModuleInfo get_module_info(const std::string& module = "");

		uint64_t read_instruction(uintptr_t address, int opcode_size, int opcode_length);

		bool load_module(int module);
		bool unload_module(int module);

		bool load_module(const std::string& module);
		bool unload_module(const std::string& module);

		bool is_loaded(int module);

		uint64_t get_foreground_user();

		template<typename T = uintptr_t>
		T get_absolute_address(uintptr_t address, uintptr_t base = -1)
		{
			if (base == -1)
				base = get_process_base();

			return reinterpret_cast<T>(base + address);
		}

		template<typename T = uintptr_t>
		T get_module_address(const std::string& module, uint64_t offset = 0)
		{
			auto library = sceKernelLoadStartModule(module.c_str(), 0, nullptr, 0, nullptr, nullptr);
			if (library < 0)
			{
				printf("sceKernelLoadStartModule(%s) failed: 0x%08X\n", module.c_str(), library);
				return 0;
			}

			SceKernelModuleInfo moduleInfo;
			moduleInfo.size = sizeof(SceKernelModuleInfo);
			if (sceKernelGetModuleInfo(library, &moduleInfo) < 0)
			{
				printf("sceKernelGetModuleInfo(%s) failed\n", module.c_str());
				return 0;
			}

			return reinterpret_cast<T>(reinterpret_cast<uintptr_t>(moduleInfo.segmentInfo->address) + offset);
		}

		template<typename T = uintptr_t>
		T get_module_address(const std::string& module, const std::string& function)
		{
			return (T)get_export_address(module, function);
		}

		template<typename T = uintptr_t>
		T get_export_address(const std::string& module, uint64_t offset)
		{
			return get_module_address<T>(module, offset);
		}
	}
}