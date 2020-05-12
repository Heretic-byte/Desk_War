#include "USB_GameManager.h"
#include "Actors/USB_PlayerController.h"
#include "Actors/USB_PlayerPawn.h"
#include "Managers/ConnectablePawnManager.h"
#include "Datas/USB_Macros.h"
#include "Engine/DataTable.h"
#include "ConstructorHelpers.h"

UUSB_GameManager::UUSB_GameManager()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> FoundTable(TEXT("DataTable'/Game/Datas/ConnectPawnTable.ConnectPawnTable'"));
	if (FoundTable.Succeeded())
	{
		m_PawnDataTable = FoundTable.Object;
	}
	else
	{
		check(FoundTable.Object);
	}
}

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
	if (!m_PawnDataTable)
	{
		PRINTF("GameMode - No Pawn Table");
		return;
	}
	m_ConnectPawnManager = new ConnectablePawnManager();
	m_ConnectPawnManager->Init(GetWorld(), m_PawnDataTable);
}

void UUSB_GameManager::Shutdown()
{
	Super::Shutdown();
	delete m_LevelManager;
	delete m_SteamManager;
	delete m_ConnectPawnManager;
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

const FConnectablePawn_Data & UUSB_GameManager::GetConnectPawnData(FName id)
{
	return m_ConnectPawnManager->GetConnectPawnData(id);
}
