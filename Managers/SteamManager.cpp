#include "SteamManager.h"
#include "Steamworks/Steamv139/sdk/public/steam/steam_api.h"
#include "Datas/USB_Macros.h"



SteamManager::SteamManager()
{
}

SteamManager::~SteamManager()
{
}

void SteamManager::Init()
{
	if (SteamAPI_Init())
	{
		PRINTF("InitSuccess");
		//const char* name = SteamFriends()->GetPersonaName();
		//PRINTF("Name:%s", *name);
	}
	else
	{
		PRINTF("InitFail");
	}
}
