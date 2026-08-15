#pragma once
#ifndef HARMONY_SETTINGS_H
#define HARMONY_SETTINGS_H

#include <atomic>

inline std::atomic_bool running{true};
inline constexpr int packageSize = 1024;
//"harmony.linush.org";
inline constexpr std::string serverURL = "127.0.0.1";
inline constexpr int serverPort = 8001;
inline constexpr int ownPort = 8000;

inline constexpr int fontQualityInPixel = 1024;


#endif
