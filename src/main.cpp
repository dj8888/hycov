#include <optional>

#include "dispatchers.hpp"
#include "globaleventhook.hpp"

#include <hyprland/src/config/ConfigValue.hpp>
#include <hyprland/src/config/values/types/IntValue.hpp>
#include <hyprland/src/config/values/types/StringValue.hpp>

APICALL EXPORT std::string PLUGIN_API_VERSION() { return HYPRLAND_API_VERSION; }

// V1 (HyprlandAPI::addConfigValue/getConfigValue) only works when Hyprland's
// config manager type is CONFIG_LEGACY (classic .conf). This setup uses the
// Lua config manager, under which V1 unconditionally returns null/false, so
// plugin config must go through V2 (addConfigValueV2 + Config::Values::*).
static Config::INTEGER registerHycovInt(const char* name, Config::INTEGER def) {
	auto val = makeShared<Config::Values::CIntValue>(name, name, def);
	HyprlandAPI::addConfigValueV2(PHANDLE, val);
	return val->value();
}

static std::string registerHycovString(const char* name, const char* def) {
	auto val = makeShared<Config::Values::CStringValue>(name, name, def);
	HyprlandAPI::addConfigValueV2(PHANDLE, val);
	return val->value();
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle)
{
	PHANDLE = handle;

	// int value
	g_hycov_overview_gappo = registerHycovInt("plugin:hycov:overview_gappo", 60);
	g_hycov_overview_gappi = registerHycovInt("plugin:hycov:overview_gappi", 24);
	g_hycov_hotarea_size = registerHycovInt("plugin:hycov:hotarea_size", 10);
	g_hycov_hotarea_pos = registerHycovInt("plugin:hycov:hotarea_pos", 1);
	g_hycov_enable_hotarea = registerHycovInt("plugin:hycov:enable_hotarea", 1);
	g_hycov_swipe_fingers = registerHycovInt("plugin:hycov:swipe_fingers", 4);
	g_hycov_move_focus_distance = registerHycovInt("plugin:hycov:move_focus_distance", 100);
	g_hycov_enable_gesture = registerHycovInt("plugin:hycov:enable_gesture", 0);
	g_hycov_disable_workspace_change = registerHycovInt("plugin:hycov:disable_workspace_change", 1);
	g_hycov_disable_spawn = registerHycovInt("plugin:hycov:disable_spawn", 0);
	g_hycov_auto_exit = registerHycovInt("plugin:hycov:auto_exit", 1);
	g_hycov_auto_fullscreen = registerHycovInt("plugin:hycov:auto_fullscreen", 0);
	g_hycov_only_active_workspace = registerHycovInt("plugin:hycov:only_active_workspace", 0);
	g_hycov_only_active_monitor = registerHycovInt("plugin:hycov:only_active_monitor", 0);
	g_hycov_enable_alt_release_exit = registerHycovInt("plugin:hycov:enable_alt_release_exit", 0);
	g_hycov_alt_toggle_auto_next = registerHycovInt("plugin:hycov:alt_toggle_auto_next", 0);
	g_hycov_click_in_cursor = registerHycovInt("plugin:hycov:click_in_cursor", 1);
	g_hycov_height_of_titlebar = registerHycovInt("plugin:hycov:height_of_titlebar", 0);
	g_hycov_show_special = registerHycovInt("plugin:hycov:show_special", 0);
	g_hycov_enable_click_action = registerHycovInt("plugin:hycov:enable_click_action", 1);
	g_hycov_raise_float_to_top = registerHycovInt("plugin:hycov:raise_float_to_top", 1);
	g_hycov_scrolling_guard_activewindow = registerHycovInt("plugin:hycov:scrolling_guard_activewindow", 1);
	g_hycov_scrolling_failsafe = registerHycovInt("plugin:hycov:scrolling_failsafe", 1);

	// string value
	g_hycov_alt_replace_key = registerHycovString("plugin:hycov:alt_replace_key", "Alt_L");
	g_hycov_hotarea_monitor = registerHycovString("plugin:hycov:hotarea_monitor", "all");

	// Hyprland's own core config, read directly (config-manager-agnostic,
	// unlike the old HyprlandAPI::getConfigValue path).
	static auto PBORDERSIZE = CConfigValue<Config::INTEGER>("general:border_size");
	static auto PGROUPBARHEIGHT = CConfigValue<Config::INTEGER>("group:groupbar:height");
	static auto PLAYOUTNAME = CConfigValue<std::string>("general:layout");

	g_hycov_bordersize = *PBORDERSIZE;
	g_hycov_groupBarHeight = *PGROUPBARHEIGHT;
	g_hycov_configLayoutName = *PLAYOUTNAME;

	g_hycov_OvGridLayout = std::make_unique<OvGridLayout>();

	// Register dispatchers FIRST so they're available even if hooks fail
	registerDispatchers();
	
	// Then register event hooks (may fail on some functions)
	try {
		registerGlobalEventHook();
	} catch (const std::exception& e) {
		HyprlandAPI::addNotification(PHANDLE, std::string("[hycov] Warning: Some hooks failed to register: ") + e.what(), CHyprColor{1.0, 0.5, 0.0, 1.0}, 5000);
	}

 	HyprlandAPI::reloadConfig();
	return {"hycov", "overview mode", "DreamMaoMao", "0.3"};
}

APICALL EXPORT void PLUGIN_EXIT() {}
