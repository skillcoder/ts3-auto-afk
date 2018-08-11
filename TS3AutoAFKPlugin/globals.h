/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/

#pragma once
#include <string>
#include "ts3_functions.h"

#define PLUGIN_NAME "AutoAFK"
#define PLUGIN_VERSION "1.1.3"
#define PLUGIN_AUTHOR "skillcoder"
#define PLUGIN_CREDITS "(McSimp, WildOrangutan)"
#define PLUGIN_DESCRIPTION "This plugin will automatically set your status to AFK mute your sound, and mute your microphone if you don't move your mouse or press any keys for %lu seconds (default). Currently working only for first %d connections."

#define DEFAULTSECONDSFORIDLE 180
#define DEFAULTMANIPULATEMIC false
#define DEFAULTMANIPULATESOUND false
#define DEFAULTDISABLEWHENFULLSCREEN true

namespace Globals {
	extern TS3Functions ts3Functions;
	extern char* pluginID;

	extern uint16_t secondsForIdle;
	extern bool manipulateMic;
	extern bool manipulateSound;
	extern bool disableWhenFullscreen;

	void loadConfig();

	std::string getConfigFilePath();
}