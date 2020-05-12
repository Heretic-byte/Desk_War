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

}

void AUSB_GameMode::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName,Options,ErrorMessage);


}

void AUSB_GameMode::StartPlay()
{
	Super::StartPlay();//all actor beginplay

	PRINTF("GameMode - StartPlay");
}

void AUSB_GameMode::EndPlay(const EEndPlayReason::Type endReason)
{
	Super::EndPlay(endReason);
}

