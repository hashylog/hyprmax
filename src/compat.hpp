#pragma once

#include <functional>
#include <string>

#include <hyprland/src/config/ConfigDataValues.hpp>
#include <hyprland/src/config/ConfigValue.hpp>
#include <hyprland/src/desktop/reserved/ReservedArea.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/helpers/Monitor.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

#if __has_include(<hyprland/src/event/EventBus.hpp>)
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/helpers/signal/Signal.hpp>
#define HYPRMAX_HAS_EVENT_BUS 1
#else
#define HYPRMAX_HAS_EVENT_BUS 0
#endif

namespace Hyprmax::Compat {

inline PHLWINDOW focusedWindow() {
    return Desktop::focusState()->window();
}

inline bool isMappedWindow(const PHLWINDOW& window) {
    return window && window->m_isMapped;
}

inline PHLMONITOR windowMonitor(const PHLWINDOW& window) {
    if (!window)
        return nullptr;

    return window->m_monitor.lock();
}

inline Vector2D windowGoalPosition(const PHLWINDOW& window) {
    return window->m_realPosition->goal();
}

inline Vector2D windowGoalSize(const PHLWINDOW& window) {
    return window->m_realSize->goal();
}

inline void dispatch(const std::string& name, const std::string& args) {
    g_pKeybindManager->m_dispatchers.at(name)(args);
}

inline void setWindowTag(const PHLWINDOW& window, const char* operation) {
    window->m_ruleApplicator->m_tagKeeper.applyTag(operation);
    window->m_ruleApplicator->propertiesChanged(Desktop::Rule::RULE_PROP_TAG);
}

inline CBox maximizedBox(const PHLWINDOW& window) {
    const auto monitor = windowMonitor(window);
    if (!monitor)
        return {};

    static auto gapsOutData = CConfigValue<Hyprlang::CUSTOMTYPE>("general:gaps_out");
    const auto* gapsOut     = sc<CCssGapData*>((gapsOutData.ptr())->getData());
    const auto  workArea    = monitor->logicalBoxMinusReserved();

    const Desktop::CReservedArea gapsReserved{
        static_cast<double>(gapsOut->m_top),
        static_cast<double>(gapsOut->m_right),
        static_cast<double>(gapsOut->m_bottom),
        static_cast<double>(gapsOut->m_left),
    };

    return gapsReserved.apply(workArea);
}

#if HYPRMAX_HAS_EVENT_BUS
using CloseWindowHook = CHyprSignalListener;

inline void registerCloseWindowHook(CloseWindowHook& hook, std::function<void(PHLWINDOW)> callback) {
    hook = Event::bus()->m_events.window.close.listen(std::move(callback));
}

inline void clearCloseWindowHook(CloseWindowHook& hook) {
    hook.reset();
}
#else
using CloseWindowHook = SP<HOOK_CALLBACK_FN>;

inline void registerCloseWindowHook(HANDLE handle, CloseWindowHook& hook, std::function<void(PHLWINDOW)> callback) {
    hook = HyprlandAPI::registerCallbackDynamic(handle, "closeWindow", [callback = std::move(callback)](void*, SCallbackInfo&, std::any data) {
        callback(std::any_cast<PHLWINDOW>(data));
    });
}

inline void clearCloseWindowHook(CloseWindowHook& hook) {
    hook.reset();
}
#endif

}
