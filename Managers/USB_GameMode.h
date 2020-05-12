// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "USB_GameMode.generated.h"


class ConnectablePawnManager;
class UDataTable;
struct FConnectablePawn_Data;

UCLASS()
class DESK_WAR_API AUSB_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AUSB_GameMode(const FObjectInitializer& ObjectInitializer);


protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)override;

	virtual void StartPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type endReason) override;


};
