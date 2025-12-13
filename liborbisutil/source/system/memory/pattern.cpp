#include "../../stdafx.h"
#include "pattern.h"

namespace liborbisutil
{
	namespace memory
	{
        std::pair<uintptr_t, size_t> get_proc_range(void) {
            static std::optional<std::pair<uintptr_t, size_t>> range = std::nullopt;
            if (range) {
                return *range;
            }

            SceKernelModuleInfo processModuleInfo;
            processModuleInfo.size = sizeof(SceKernelModuleInfo);
            if (sceKernelGetModuleInfo(0, &processModuleInfo) < 0)
            {
                return *range;
            }

            uintptr_t base = reinterpret_cast<uintptr_t>(processModuleInfo.segmentInfo[0].address);
            size_t size = 0;
            for (int i = 0; i < processModuleInfo.segmentCount; i++)
            {
                size += processModuleInfo.segmentInfo[i].size;
            }

            range = std::make_pair(base, size);
            return *range;
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

        uintptr_t sigscan(const uint8_t* bytes, const uint8_t* mask, size_t pattern_size)
        {
            auto [start, size] = get_proc_range();
            return sigscan(bytes, mask, pattern_size, start, size);
        }
	}
}