#include "USB_GameMode.h"
#include "Actors/USB_PlayerController.h"
#include "Actors/USB_PlayerPawn.h"
#include "Managers/ConnectablePawnManager.h"
#include "Datas/USB_Macros.h"

AUSB_GameMode::AUSB_GameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bNetLoadOnClient = false;
	bPauseable = true;
	bStartPlayersAsSpectators = false;

	DefaultPawnClass = AUSB_PlayerPawn::StaticClass();
	PlayerControllerClass = AUSB_PlayerController::StaticClass();
	//
	m_ConnectPawnManager = nullptr;
}

void AUSB_GameMode::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName,Options,ErrorMessage);

	if (!m_PawnDataTable)
	{
		PRINTF("GameMode - No Pawn Table");
		return;
	}
	m_ConnectPawnManager = new ConnectablePawnManager();
	m_ConnectPawnManager->Init(GetWorld(),m_PawnDataTable);
}

void AUSB_GameMode::StartPlay()
{
	Super::StartPlay();//all actor beginplay

	PRINTF("GameMode - StartPlay");
}

void AUSB_GameMode::EndPlay(const EEndPlayReason::Type endReason)
{
	Super::EndPlay(endReason);
	delete m_ConnectPawnManager;
}

const FConnectablePawn_Data & AUSB_GameMode::GetConnectPawnData(FName id)
{
	return m_ConnectPawnManager->GetConnectPawnData(id);
}
