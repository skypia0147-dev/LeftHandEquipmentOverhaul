#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

#include <spdlog/sinks/basic_file_sink.h>

// Windows.h after CommonLib (REX conflicts otherwise)
#include <Windows.h>

using namespace std::literals;
namespace logger = SKSE::log;
