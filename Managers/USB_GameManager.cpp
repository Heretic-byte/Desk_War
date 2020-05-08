#include "USB_GameManager.h"


void UUSB_GameManager::GameOver()
{
	PRINTF("GameOver");
}

void UUSB_GameManager::Init()
{
	Super::Init();
	m_LevelManager = new LevelManager();
	m_LevelManager->Init(this);
	m_SteamManager = new SteamManager();
	m_SteamManager->Init();
}

void UUSB_GameManager::Shutdown()
{
	Super::Shutdown();
	delete m_LevelManager;
	delete m_SteamManager;
}

void UUSB_GameManager::UnloadCurrent()
{
	m_LevelManager->UnloadCurrent();
}

void UUSB_GameManager::UnloadLevel(FName wantLevel)
{
	m_LevelManager->UnloadLevel(wantLevel);
}

void UUSB_GameManager::LoadLevel(FName newLevelName)
{
	m_LevelManager->LoadLevel(newLevelName);
}
