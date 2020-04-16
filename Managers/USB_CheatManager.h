// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "Managers/USB_GameManager.h"
#include "Actors/USB_PlayerPawn.h"
#include "Actors/USB_PlayerController.h"
#include "USB_CheatManager.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API UUSB_CheatManager : public UCheatManager
{
	GENERATED_BODY()
protected:
	UPROPERTY()
	AUSB_PlayerController* m_PlayerController;
	UPROPERTY()
	AUSB_PlayerPawn* m_Player;
	UPROPERTY()
	UUSB_GameManager* m_Manager;
protected:
	virtual void InitCheatManager() override;
public:
	UFUNCTION(exec)
	void PrintPlayerVelocity();
	UFUNCTION(exec)
	void HidePlayerVelocity();
	UFUNCTION(exec)
	void PrintPlayerMass();
	bool m_bShowVelo;
};
