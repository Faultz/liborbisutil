#pragma once

// ============================================================================
// liborbisutil - PlayStation 4 Utility Library
// ============================================================================
// Include the precompiled header for all standard includes
#include "../source/stdafx.h"

// ============================================================================
// System Modules
// ============================================================================
#include "../source/system/string/string.h"
#include "../source/system/logger/logger.h"
#include "../source/system/resolve/resolve.h"
#include "../source/system/file/filesystem.h"
#include "../source/system/thread/thread.h"
#include "../source/system/memory/memory.h"
#include "../source/system/memory/allocation/memory_allocators.h"
#include "../source/system/patcher/patcher.h"
#include "../source/system/math/math.h"
#include "../source/system/pad/pad.h"

// ============================================================================
// Audio
// ============================================================================
#include "../source/system/audio/wav.h"
#include "../source/system/audio/player.h"

// ============================================================================
// Hooking
// ============================================================================
#include "../source/hooking/manager.h"

// ============================================================================
// Network
// ============================================================================
#include "../source/network/http.h"
#include "../source/network/sockets.h"