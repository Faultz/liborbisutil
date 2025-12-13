// liborbisutil precompiled header
// Include this file first in all .cpp files
#pragma once

// ============================================================================
// C Standard Library
// ============================================================================
#include <stdio.h>
#include <cstdint>
#include <csetjmp>

// ============================================================================
// C++ Standard Library
// ============================================================================
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <memory>
#include <functional>

// Threading
#include <mutex>
#include <thread>
#include <atomic>
#include <future>
#include <condition_variable>
#include <queue>

// Containers
#include <set>
#include <unordered_map>

// ============================================================================
// PlayStation 4 SDK Headers
// ============================================================================
#include <kernel.h>
#include <KernelExt.h>
#include <libsysmodule.h>

#include <gnmx.h>

#include <pad.h>

#include <net.h>
#include <libhttp.h>
#include <rtcExt.h>

// ============================================================================
// Third-party Libraries
// ============================================================================
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/os.h>

#include <minhook.h>

// ============================================================================
// Library Linkage
// ============================================================================
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
