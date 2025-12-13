#include "resolve.h"

#include "UserServiceExt.h"

namespace liborbisutil
{
	namespace resolve
	{
		uintptr_t get_process_base()
		{
			SceKernelModuleInfo processModuleInfo;
			processModuleInfo.size = sizeof(SceKernelModuleInfo);
			if (sceKernelGetModuleInfo(0, &processModuleInfo) < 0)
			{
				printf("sceKernelGetModuleInfo failed\n");
				return 0;
			}

			return reinterpret_cast<uintptr_t>(processModuleInfo.segmentInfo[0].address);
		}

		uintptr_t get_export_address(const std::string& module, const std::string& function)
		{
			auto library = sceKernelLoadStartModule(module.c_str(), 0, nullptr, 0, nullptr, nullptr);
			if (library < 0)
			{
				printf("sceKernelLoadStartModule(%s) failed: 0x%08X\n", module.c_str(), library);
				return 0;
			}

			void* address = nullptr;
			if(sceKernelDlsym(library, function.c_str(), &address) < 0)
			{
				printf("sceKernelDlsym(%s) failed\n", function.c_str());
				return 0;
			}

			return reinterpret_cast<uintptr_t>(address);
		}

		size_t get_module_size(const std::string& module)
		{
			size_t size = 0;
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

			for (int i = 0; i < moduleInfo.segmentCount; i++)
			{
				size += moduleInfo.segmentInfo[i].size;
			}

			return size;
		}

		SceKernelModuleInfo get_module_info(const std::string& module)
		{
			SceKernelModuleInfo moduleInfo;
			moduleInfo.size = sizeof(SceKernelModuleInfo);

			if (module.empty())
			{
				if (sceKernelGetModuleInfo(0, &moduleInfo) < 0)
				{
					printf("sceKernelGetModuleInfo failed\n");
				}

				return moduleInfo;
			}

			auto library = sceKernelLoadStartModule(module.c_str(), 0, nullptr, 0, nullptr, nullptr);
			if (library < 0)
			{
				printf("sceKernelLoadStartModule(%s) failed: 0x%08X\n", module.c_str(), library);
				return moduleInfo;
			}

			if (sceKernelGetModuleInfo(library, &moduleInfo) < 0)
			{
				printf("sceKernelGetModuleInfo(%s) failed\n", module.c_str());
				return moduleInfo;
			}

			return moduleInfo;
		}

		uint64_t read_instruction(uintptr_t address, int opcode_size, int opcode_length)
		{
			return (uint64_t)(*(int*)(address + opcode_size) + address + opcode_length);
		}

		bool load_module(int module)
		{
			return sceSysmoduleLoadModule(module) >= 0;
		}

		bool unload_module(int module)
		{
			return sceSysmoduleUnloadModule(module) >= 0;
		}

		bool load_module(const std::string& module)
		{
			auto library = sceKernelLoadStartModule(module.data(), 0, nullptr, 0, nullptr, nullptr);
			if (library < 0)
			{
				printf("sceKernelLoadStartModule(%s) failed: 0x%X\n", module.data(), library);
				return false;
			}

			return true;
		}

		bool unload_module(const std::string& module)
		{
			return true;
		}

		bool is_loaded(int module)
		{
			return sceSysmoduleIsLoaded(module) >= 0;
		}

		uint64_t get_foreground_user()
		{
			int userId;
			sceUserServiceGetForegroundUser(&userId);
			return userId;
		}
	}
}