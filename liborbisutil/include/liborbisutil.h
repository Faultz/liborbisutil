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

#define LIBORBISUTIL_HTTP_CTX_DEFAULT -1

using posix_thread = thread;

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