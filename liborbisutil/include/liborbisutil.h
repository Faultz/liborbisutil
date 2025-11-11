#pragma once

#include <stdio.h>
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <mutex>
#include <set>
#include <unordered_map>
#include <memory>
#include <functional>
#include <queue>
#include <condition_variable>
#include <future>
#include <csetjmp>

#include <gnmx.h>
#include <kernel.h>
#include <KernelExt.h>

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/os.h>

#include <libhttp.h>
#include <net.h>

#include <pad.h>

#pragma comment(lib, "libSceLibcInternal_gen_stub_weak.a")
#pragma comment(lib, "libkernel_gen_stub_weak.a")
#pragma comment(lib, "libSceRtc_gen_stub_weak.a")
#pragma comment(lib, "libSceSystemStateMgr_gen_stub_weak.a")
#pragma comment(lib, "libSceSystemService_gen_stub_weak.a")
#pragma comment(lib, "libSceSysCore_gen_stub_weak.a")
#pragma comment(lib, "libSceUserService_gen_stub_weak.a")
#pragma comment(lib, "libSceNet_gen_stub_weak.a")
#pragma comment(lib, "libSceLncUtil_gen_stub_weak.a")
#pragma comment(lib, "libScePad_stub_weak.a")
#pragma comment(lib, "libSceRtc_stub_weak.a")
#pragma comment(lib, "libSceVideoOut_stub_weak.a")
#pragma comment(lib, "libSceDbg_stub_weak.a")
#pragma comment(lib, "libSceNpWebApi_stub_weak.a")
#pragma comment(lib, "libSceNpAuth_stub_weak.a")
#pragma comment(lib, "libSceNpManager_stub_weak.a")
#pragma comment(lib, "libSceImeDialog_stub_weak.a")
#pragma comment(lib, "libSceAudioOut_stub_weak.a")
#pragma comment(lib, "libSceAudioIn_stub_weak.a")
#pragma comment(lib, "libSceVoice_stub_weak.a")
#pragma comment(lib, "libSceSysmodule_stub_weak.a")
#pragma comment(lib, "libSceHttp_stub_weak.a")
#pragma comment(lib, "libSceSsl_stub_weak.a")
#pragma comment(lib, "libSceNet_stub_weak.a")
#pragma comment(lib, "libSceGameLiveStreaming_stub_weak.a")
#pragma comment(lib, "libSceGnm.a")
#pragma comment(lib, "libSceGnmx.a")
#pragma comment(lib, "libSceGnmDriver_stub_weak.a")
#pragma comment(lib, "libSceGnf.a")
#pragma comment(lib, "libSceGpuAddress.a")
#pragma comment(lib, "libSceDbgVideoOut_stub_weak.a")
#pragma comment(lib, "libScePm4Dump.a")
#pragma comment(lib, "fmtlib.a")
#pragma comment(lib, "libMinhook.a")

#include "../source/system/logger.h"
#include "../source/system/resolve.h"
#include "../source/system/filesystem.h"
#include "../source/system/string.h"
#include "../source/system/thread.h"
#include "../source/system/memory.h"
#include "../source/system/memory_allocators.h"
#include "../source/system/patcher.h"
#include "../source/system/pad.h"

#include "../source/hooking/manager.h"

#include "../source/network/http.h"
#include "../source/network/sockets.h"

#include "../source/math.h"

#include "../source/system/audio/wav.h"
#include "../source/system/audio/player.h"