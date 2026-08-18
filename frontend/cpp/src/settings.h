#pragma once
#ifndef HARMONY_SETTINGS_H
#define HARMONY_SETTINGS_H

#include <atomic>

inline std::atomic_bool running{true};
inline constexpr int packageSize = 1024;
//"harmony.linush.org";
inline const std::string serverURL = "https://harmony.timpagels.de";
inline const std::string wsServerURL = "wss://harmony.timpagels.de";

inline constexpr int fontQualityInPixel = 1024;


#endif
