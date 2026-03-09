#define WLR_USE_UNSTABLE

#include <format>
#include <stdexcept>
#include <string>

#include "compat.hpp"
#include "globals.hpp"

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

static void applyMaximize(PHLWINDOW window) {
    const auto targetBox = Hyprmax::Compat::maximizedBox(window);
    if (targetBox.w <= 0 || targetBox.h <= 0)
        return;

    Hyprmax::Compat::dispatch("resizeactive", std::format("exact {} {}", static_cast<int>(targetBox.w), static_cast<int>(targetBox.h)));
    Hyprmax::Compat::dispatch("moveactive", std::format("exact {} {}", static_cast<int>(targetBox.x), static_cast<int>(targetBox.y)));
    Hyprmax::Compat::setWindowTag(window, "+hyprmax");
}

static SDispatchResult onHyprmax(std::string args) {
    (void)args;

    const auto PWINDOW = Hyprmax::Compat::focusedWindow();

    if (!Hyprmax::Compat::isMappedWindow(PWINDOW))
        return {.success = false, .error = "No valid window"};

    auto it = g_windowStates.find(PWINDOW);
    if (it != g_windowStates.end()) {
        const auto saved = it->second;
        g_windowStates.erase(it);

        Hyprmax::Compat::dispatch("resizeactive", std::format("exact {} {}", static_cast<int>(saved.size.x), static_cast<int>(saved.size.y)));
        Hyprmax::Compat::dispatch("moveactive", std::format("exact {} {}", static_cast<int>(saved.position.x), static_cast<int>(saved.position.y)));
        Hyprmax::Compat::setWindowTag(PWINDOW, "-hyprmax");

        return {};
    }

    if (!PWINDOW->m_isFloating) {
        Hyprmax::Compat::dispatch("setfloating", "");
        return {};
    }

    g_windowStates[PWINDOW] = SWindowState{
        .position = Hyprmax::Compat::windowGoalPosition(PWINDOW),
        .size     = Hyprmax::Compat::windowGoalSize(PWINDOW),
    };

    applyMaximize(PWINDOW);
    return {};
}

static Hyprmax::Compat::CloseWindowHook g_closeWindowHook;

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH        = __hyprland_api_get_hash();
    const std::string CLIENT_HASH = __hyprland_api_get_client_hash();

    if (HASH != CLIENT_HASH) {
        HyprlandAPI::addNotification(PHANDLE, "[hyprmax] Version mismatch (headers != running Hyprland)", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[hyprmax] Version mismatch");
    }

    bool success = HyprlandAPI::addDispatcherV2(PHANDLE, "hyprmax", ::onHyprmax);

#if HYPRMAX_HAS_EVENT_BUS
    Hyprmax::Compat::registerCloseWindowHook(g_closeWindowHook, [](PHLWINDOW window) {
        g_windowStates.erase(window);
    });
#else
    Hyprmax::Compat::registerCloseWindowHook(PHANDLE, g_closeWindowHook, [](PHLWINDOW window) {
        g_windowStates.erase(window);
    });
#endif

    if (success)
        HyprlandAPI::addNotification(PHANDLE, "[hyprmax] Initialized successfully!", CHyprColor{0.2, 1.0, 0.2, 1.0}, 5000);
    else {
        HyprlandAPI::addNotification(PHANDLE, "[hyprmax] Failed to register dispatcher", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[hyprmax] Dispatcher registration failed");
    }

    return {"hyprmax", "Fake maximize for floating WM that avoids z-order bugs", "hashylog", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    Hyprmax::Compat::clearCloseWindowHook(g_closeWindowHook);
    g_windowStates.clear();
}
