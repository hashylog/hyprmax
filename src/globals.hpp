#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <unordered_map>

inline HANDLE PHANDLE = nullptr;

struct SWindowState {
    Vector2D position;
    Vector2D size;
};

inline std::unordered_map<PHLWINDOW, SWindowState> g_windowStates;
