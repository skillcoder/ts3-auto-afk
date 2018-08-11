#include <iostream>
#include <thread>
#include <Windows.h>
#include <inttypes.h>

#include "plugin.hpp"
#include "globals.h"
#include "qtConfigDialog.h"

using namespace std;
using namespace Globals;

#define _DEBUG true

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#if _DEBUG
#define debuglog(str, ...) printf(str, __VA_ARGS__);
#else
#define debuglog(str, ...)
#endif

#define MAXCONNECTEDSERVERS 32

//static struct TS3Functions ts3;
static bool shouldExitMonitorThread;
static std::thread idleMonitorThread;
static bool isSetToAFK;

static uint64 connectedServers[MAXCONNECTEDSERVERS];
static bool shouldRestoreAFK[MAXCONNECTEDSERVERS];
static bool shouldRestoreMuteSound[MAXCONNECTEDSERVERS];
static bool shouldRestoreMuteMic[MAXCONNECTEDSERVERS];

static char *description = NULL;

const char* ts3plugin_name()	
{
	return PLUGIN_NAME;
}

const char* ts3plugin_version()
{
    return PLUGIN_VERSION;
}

int ts3plugin_apiVersion()
{
	return PLUGIN_API_VERSION;
}

const char* ts3plugin_author()
{
	return PLUGIN_CREDITS " " PLUGIN_AUTHOR;
}

const char* ts3plugin_description()
{	
	size_t needed = snprintf(NULL, 0, PLUGIN_DESCRIPTION, secondsForIdle, MAXCONNECTEDSERVERS) + 1;
	
	description = (char*)std::malloc(needed);
	if (description) {
		snprintf(description, needed, PLUGIN_DESCRIPTION, secondsForIdle, MAXCONNECTEDSERVERS);
		return (const char*) description;
	}
	else {
		return "description malloc failed";
	}
}

/*
* If the plugin wants to use error return codes, plugin commands, hotkeys or menu items, it needs to register a command ID. This function will be
* automatically called after the plugin was initialized. This function is optional. If you don't use these features, this function can be omitted.
* Note the passed pluginID parameter is no longer valid after calling this function, so you must copy it and store it in the plugin.
*/
void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);  /* The id buffer will invalidate after exiting this function */
	printf("[AutoAFK] registerPluginID: %s\n", pluginID);
}

void ts3plugin_setFunctionPointers(const struct TS3Functions funcs)
{
	ts3Functions = funcs;
}

bool IsTopMost( HWND hwnd )
{
  WINDOWINFO info;
  GetWindowInfo( hwnd, &info );
  return ( info.dwExStyle & WS_EX_TOPMOST ) ? true : false;
}

bool IsFullScreenSize( HWND hwnd, const int cx, const int cy )
{
  RECT r;
  ::GetWindowRect( hwnd, &r );
  return r.right - r.left == cx && r.bottom - r.top == cy;
}

bool IsFullscreenAndMaximized( HWND hwnd )
{
  if( IsTopMost( hwnd ) )
  {
    const int cx = GetSystemMetrics( SM_CXSCREEN );
    const int cy = GetSystemMetrics( SM_CYSCREEN );
    if( IsFullScreenSize( hwnd, cx, cy ) )
      return true;
  }
  return false;
}

BOOL CALLBACK CheckMaximized( HWND hwnd, LPARAM lParam )
{
  if( IsFullscreenAndMaximized( hwnd ) )
  {
    * (bool*) lParam = true;
    return FALSE; //there can be only one so quit here
  }
  return TRUE;
}

bool isSmthFullscreen() {
	bool bThereIsAFullscreenWin = false;
	EnumWindows( (WNDENUMPROC) CheckMaximized, (LPARAM) &bThereIsAFullscreenWin );

	return bThereIsAFullscreenWin;
}

void setToAFK(int connectionIdx)
{
	uint64 serverID = connectedServers[connectionIdx];
	if (manipulateMic) {
		// When we're setting the person as AFK, if they already have some kind of status
		// set on their mic (muted for example), then don't unmute it when they come back.
		int micMuted;
		ts3Functions.getClientSelfVariableAsInt(serverID, CLIENT_INPUT_MUTED, &micMuted);

		if (micMuted != MUTEINPUT_NONE)
		{
			shouldRestoreMuteMic[connectionIdx] = false;
		}
		else
		{
			shouldRestoreMuteMic[connectionIdx] = true;
			ts3Functions.setClientSelfVariableAsInt(serverID, CLIENT_INPUT_MUTED, MUTEINPUT_MUTED);
		}
	}

	if (manipulateSound) {
		// Same goes for speakers
		int outMuted;
		ts3Functions.getClientSelfVariableAsInt(serverID, CLIENT_OUTPUT_MUTED, &outMuted);

		if (outMuted != MUTEOUTPUT_NONE)
		{
			shouldRestoreMuteSound[connectionIdx] = false;
		}
		else
		{
			shouldRestoreMuteSound[connectionIdx] = true;
			ts3Functions.setClientSelfVariableAsInt(serverID, CLIENT_OUTPUT_MUTED, MUTEOUTPUT_MUTED);
		}
	}
	// And AFK status
	int afkStatus;
	ts3Functions.getClientSelfVariableAsInt(serverID, CLIENT_AWAY, &afkStatus);

	if(afkStatus != AWAY_NONE)
	{
		shouldRestoreAFK[connectionIdx] = false;
	}
	else
	{
		shouldRestoreAFK[connectionIdx] = true;
		ts3Functions.setClientSelfVariableAsInt(serverID, CLIENT_AWAY, AWAY_ZZZ);
	}

	ts3Functions.flushClientSelfUpdates(serverID, NULL);

	//std::cout << "setTOAFK: " << shouldRestoreAFK << shouldRestoreMuteMic << shouldRestoreMuteSound << std::endl; 
}

void setBack(int connectionIdx)
{
	uint64 serverID = connectedServers[connectionIdx];
	// Set AFK status
	if(shouldRestoreAFK[connectionIdx])
	{
		shouldRestoreAFK[connectionIdx] = false;
		ts3Functions.setClientSelfVariableAsInt(serverID, CLIENT_AWAY, AWAY_NONE);
	}

	if(manipulateMic)
		// Microphone toggle
		if(shouldRestoreMuteMic[connectionIdx])
		{
			shouldRestoreMuteMic[connectionIdx] = false;
			ts3Functions.setClientSelfVariableAsInt(serverID, CLIENT_INPUT_MUTED, MUTEINPUT_NONE);
		}

	if(manipulateSound)
		// Speaker toggle
		if(shouldRestoreMuteSound[connectionIdx])
		{
			shouldRestoreMuteSound[connectionIdx] = false;
			ts3Functions.setClientSelfVariableAsInt(serverID, CLIENT_OUTPUT_MUTED, MUTEOUTPUT_NONE);
		}
			
	ts3Functions.flushClientSelfUpdates(serverID, NULL);
}

void toggleAFK(bool isAFK)
{
	uint64* ids;

    if(ts3Functions.getServerConnectionHandlerList(&ids) != ERROR_ok)
	{
		ts3Functions.logMessage("[AutoAFK] Error retrieving server list - cannot set AFK", LogLevel_ERROR, "Plugin", 0);
        return;
    }

	if (isAFK) {
		printf("[AutoAFK] Set AFK\n");
	}
	else {
		printf("[AutoAFK] Set Back\n");
	}

	// Foreach connection
    for(int i = 0; ids[i]; i++) 
	{
		uint64 serverID = ids[i];
		if (i >= MAXCONNECTEDSERVERS) break;
		connectedServers[i] = serverID;
        anyID clid;
		//printf("%d => %" PRIu64 " ", i, serverID);
        if(ts3Functions.getClientID(serverID, &clid) == ERROR_ok)
		{
			if(isAFK)
			{
				setToAFK(i);
			}
			else
			{
				setBack(i);
			}
        }
    }

	//printf("\n");
	ts3Functions.freeMemory(ids);
	isSetToAFK = isAFK;
}

void idleWatcher()
{
	LASTINPUTINFO lastInput;
	lastInput.cbSize = sizeof(LASTINPUTINFO);

	std::chrono::milliseconds measureInterval(1000);
	std::chrono::milliseconds loopTime(500);
	auto start = std::chrono::steady_clock::now();

	while(!shouldExitMonitorThread)
	{
		if((std::chrono::steady_clock::now() - start) > measureInterval)
		{
			if (!(disableWhenFullscreen && isSmthFullscreen()))
				if(GetLastInputInfo(&lastInput))
				{
					DWORD awayTimeSecs = ((GetTickCount() - lastInput.dwTime)/1000);
					
					//printf("TIME %d\n", awayTimeSecs);
					if(awayTimeSecs > secondsForIdle)
					{
						if(!isSetToAFK)
							toggleAFK(true);
					}
					else if(isSetToAFK)
					{
						toggleAFK(false);
					}
				}

			start = std::chrono::steady_clock::now();
		}

		std::this_thread::sleep_for(loopTime);
	}
}

int ts3plugin_requestAutoload()
{
    return 1;
}

int ts3plugin_init()
{
	loadConfig();

	printf("%s\n", getConfigFilePath().c_str());

	shouldExitMonitorThread = false;
	idleMonitorThread = std::thread(idleWatcher);
	return 0;
}

void ts3plugin_shutdown()
{
	shouldExitMonitorThread = true;
	idleMonitorThread.join();

	/*
	* Note:
	* If your plugin implements a settings dialog, it must be closed and deleted here, else the
	* TeamSpeak client will most likely crash (DLL removed but dialog from DLL code still open).
	*/

	/* Free pluginID if we registered it */
	if (pluginID) {
		free(pluginID);
		pluginID = NULL;
	}

	//qConfigDialog.close();

	std::free(description);
	description = NULL;
}

/* Tell client if plugin offers a configuration window. If this function is not implemented, it's an assumed "does not offer" (PLUGIN_OFFERS_NO_CONFIGURE). */
int ts3plugin_offersConfigure() {
	printf("[AutoAFK] offersConfigure\n");
	/*
	* Return values:
	* PLUGIN_OFFERS_NO_CONFIGURE         - Plugin does not implement ts3plugin_configure
	* PLUGIN_OFFERS_CONFIGURE_NEW_THREAD - Plugin does implement ts3plugin_configure and requests to run this function in an own thread
	* PLUGIN_OFFERS_CONFIGURE_QT_THREAD  - Plugin does implement ts3plugin_configure and requests to run this function in the Qt GUI thread
	*/
	return PLUGIN_OFFERS_CONFIGURE_QT_THREAD;  /* In this case ts3plugin_configure does not need to be implemented */
}

/* Plugin might offer a configuration window. If ts3plugin_offersConfigure returns 0, this function does not need to be implemented. */
void ts3plugin_configure(void* handle, void* qParentWidget) {
	printf("[AutoAFK] configure\n");
	//qtConfigDialog qConfigDialog;
	//qConfigDialog.SetupUi();
	//qConfigDialog.exec();
}
