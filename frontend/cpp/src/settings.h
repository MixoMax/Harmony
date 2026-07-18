#pragma once
#include <atomic>

inline std::atomic_bool running{true};
inline constexpr int packageSize = 1024;
inline const std::string serverURL = "harmony.linush.org";

inline constexpr int fontQualityInPixel = 1024;
