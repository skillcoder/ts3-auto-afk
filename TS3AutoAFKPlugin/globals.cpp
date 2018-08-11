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

#include "globals.h"
#include <QtCore/QSettings>

namespace Globals {
	TS3Functions ts3Functions;

	char* pluginID = NULL;
	
	uint16_t secondsForIdle = DEFAULTSECONDSFORIDLE;
	bool manipulateMic = DEFAULTMANIPULATEMIC;
	bool manipulateSound = DEFAULTMANIPULATESOUND;
	bool disableWhenFullscreen = DEFAULTDISABLEWHENFULLSCREEN;

	void loadConfig() {
		QSettings cfg(QString::fromStdString(getConfigFilePath()), QSettings::IniFormat);
		secondsForIdle = cfg.value("secondsForIdle", DEFAULTSECONDSFORIDLE).toUInt();
		manipulateMic = cfg.value("manipulateMic", DEFAULTMANIPULATEMIC).toBool();
		manipulateSound = cfg.value("manipulateSound", DEFAULTMANIPULATESOUND).toBool();
		disableWhenFullscreen = cfg.value("disableWhenFullscreen", DEFAULTDISABLEWHENFULLSCREEN).toBool();
	}

	std::string getConfigFilePath() {
		char* configPath = (char*)malloc(512);
		ts3Functions.getConfigPath(configPath, 512);
		std::string path = configPath;
		free(configPath);
		path.append(PLUGIN_NAME);
		path.append(".ini");
		return path;
	}
}