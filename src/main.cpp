#define WLR_USE_UNSTABLE

#include <unistd.h>

#include <hyprland/src/includes.hpp>
#include <sstream>

#define private public
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>
#undef private

#include "globals.hpp"

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

static void applyMaximize(PHLWINDOW window) {
    const auto PMONITOR = window->m_monitor.lock();
    if (!PMONITOR)
        return;

    // Work area already excludes bars, panels, etc.
    CBox workArea = PMONITOR->logicalBoxMinusReserved();

    // Read gaps_out from config (supports CSS-style: top right bottom left)
    static auto PGAPSOUTDATA = CConfigValue<Hyprlang::CUSTOMTYPE>("general:gaps_out");
    auto*       PGAPSOUT     = sc<CCssGapData*>((PGAPSOUTDATA.ptr())->getData());

    // Apply gaps to work area
    Desktop::CReservedArea gapsReserved{(double)PGAPSOUT->m_top, (double)PGAPSOUT->m_right, (double)PGAPSOUT->m_bottom, (double)PGAPSOUT->m_left};
    CBox                   targetBox = gapsReserved.apply(workArea);

    // Resize and move via dispatchers (proper layout path)
    g_pKeybindManager->m_dispatchers["resizeactive"](std::format("exact {} {}", (int)targetBox.w, (int)targetBox.h));
    g_pKeybindManager->m_dispatchers["moveactive"](std::format("exact {} {}", (int)targetBox.x, (int)targetBox.y));

    // Tag window as fakemaxed
    window->m_ruleApplicator->m_tagKeeper.applyTag("+hyprmax");
    window->m_ruleApplicator->propertiesChanged(Desktop::Rule::RULE_PROP_TAG);
}

static SDispatchResult onHyprmax(std::string args) {
    const auto PWINDOW = Desktop::focusState()->window();

    if (!PWINDOW || !PWINDOW->m_isMapped)
        return {.success = false, .error = "No valid window"};

    // Already fakemaxed — restore original geometry
    auto it = g_windowStates.find(PWINDOW);
    if (it != g_windowStates.end()) {
        const auto saved = it->second;
        g_windowStates.erase(it);

        // Restore via dispatchers (proper layout path)
        g_pKeybindManager->m_dispatchers["resizeactive"](std::format("exact {} {}", (int)saved.size.x, (int)saved.size.y));
        g_pKeybindManager->m_dispatchers["moveactive"](std::format("exact {} {}", (int)saved.position.x, (int)saved.position.y));

        if (saved.wasTiled)
            g_pKeybindManager->m_dispatchers["settiled"]("");

        PWINDOW->m_ruleApplicator->m_tagKeeper.applyTag("-hyprmax");
        PWINDOW->m_ruleApplicator->propertiesChanged(Desktop::Rule::RULE_PROP_TAG);

        return {};
    }

    // Tiled window — just convert to floating (tiled already behaves like maximized)
    if (!PWINDOW->m_isFloating) {
        g_pKeybindManager->m_dispatchers["setfloating"]("");
        return {};
    }

    // Floating window — save geometry and maximize
    g_windowStates[PWINDOW] = SWindowState{
        .position = PWINDOW->m_realPosition->goal(),
        .size     = PWINDOW->m_realSize->goal(),
        .wasTiled = false,
    };

    applyMaximize(PWINDOW);
    return {};
}

static SP<HOOK_CALLBACK_FN> g_pCloseWindowHook;

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH        = __hyprland_api_get_hash();
    const std::string CLIENT_HASH = __hyprland_api_get_client_hash();

    if (HASH != CLIENT_HASH) {
        HyprlandAPI::addNotification(PHANDLE, "[hyprmax] Version mismatch (headers != running Hyprland)", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[hyprmax] Version mismatch");
    }

    bool success = HyprlandAPI::addDispatcherV2(PHANDLE, "hyprmax", ::onHyprmax);

    // Cleanup state when windows are destroyed
    g_pCloseWindowHook = HyprlandAPI::registerCallbackDynamic(PHANDLE, "closeWindow", [](void*, SCallbackInfo&, std::any data) {
        auto window = std::any_cast<PHLWINDOW>(data);
        g_windowStates.erase(window);
    });

    if (success)
        HyprlandAPI::addNotification(PHANDLE, "[hyprmax] Initialized successfully!", CHyprColor{0.2, 1.0, 0.2, 1.0}, 5000);
    else {
        HyprlandAPI::addNotification(PHANDLE, "[hyprmax] Failed to register dispatcher", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[hyprmax] Dispatcher registration failed");
    }

    return {"hyprmax", "Fake maximize for floating WM that avoids z-order bugs", "hashylog", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    g_windowStates.clear();
}
