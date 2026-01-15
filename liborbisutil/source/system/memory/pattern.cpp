#include "../../stdafx.h"
#include "pattern.h"
#include "../resolve/resolve.h"

namespace liborbisutil
{
    namespace memory
    {
        std::pair<uintptr_t, size_t> get_proc_or_mod_range(std::string_view name) {
			static std::unordered_map<std::string_view, std::pair<uintptr_t, size_t>> module_ranges;

			auto range_it = module_ranges.find(name);
			if (range_it != module_ranges.end()) 
            {
				return range_it->second;
			}

            if (name == "")
            {
				return { resolve::get_process_base(), resolve::get_module_size("") };
            }

            int mod_id = 0;
            if (!name.empty())
            {
                mod_id = sceKernelLoadStartModule(name.data(), 0, nullptr, 0, nullptr, nullptr);
            }

            uintptr_t base = 0;
            size_t size = 0;
            SceKernelModuleInfo processModuleInfo;
            processModuleInfo.size = sizeof(SceKernelModuleInfo);
            if (sceKernelGetModuleInfo(mod_id, &processModuleInfo) < 0) {
                return {};
            }

            base = (uintptr_t)processModuleInfo.segmentInfo[0].address;
            size = processModuleInfo.segmentInfo[0].size;

			auto pair = std::make_pair(base, size);
			auto range = module_ranges.find(name);
			if (range == module_ranges.end())
            {
				module_ranges[name] = pair;
			}

            return pair;
        }

        uintptr_t sigscan(const uint8_t* bytes,
            const uint8_t* mask,
            size_t pattern_size,
            uintptr_t start,
            size_t size) {
            auto start_ptr = reinterpret_cast<uint8_t*>(start);

            // The naive O(nm) search works well enough, even repeating it for each different pattern
            for (size_t i = 0; i < (size - pattern_size); i++) {
                bool found = true;
                for (size_t j = 0; j < pattern_size; j++) {
                    auto val = start_ptr[i + j];
                    if ((val & mask[j]) != bytes[j]) {
                        found = false;
                        break;
                    }
                }
                if (found) {
                    return reinterpret_cast<uintptr_t>(&start_ptr[i]);
                }
            }

            return 0;
        }

        uintptr_t sigscan(const uint8_t* bytes, const uint8_t* mask, size_t pattern_size, std::string_view name)
        {
            auto [start, size] = get_proc_or_mod_range(name);
            return sigscan(bytes, mask, pattern_size, start, size);
        }
    }
}