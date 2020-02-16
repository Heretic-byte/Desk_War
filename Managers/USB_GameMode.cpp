// Fill out your copyright notice in the Description page of Project Settings.


#include "USB_GameMode.h"
#include "Actors/USB_PlayerController.h"
#include "Actors/USB_PlayerPawn.h"



AUSB_GameMode::AUSB_GameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bNetLoadOnClient = false;
	bPauseable = true;
	bStartPlayersAsSpectators = false;

	DefaultPawnClass = AUSB_PlayerPawn::StaticClass();
	PlayerControllerClass = AUSB_PlayerController::StaticClass();
}