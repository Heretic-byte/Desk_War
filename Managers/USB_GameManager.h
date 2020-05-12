// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"


#include "Datas/USB_Macros.h"
#include "Managers/LevelManager.h"
#include "Managers/SteamManager.h"
#include "USB_GameManager.generated.h"

/**
 * 
 */

class ConnectablePawnManager;
class UDataTable;
struct FConnectablePawn_Data;
UCLASS()
class DESK_WAR_API UUSB_GameManager : public UGameInstance
{
	GENERATED_BODY()
public:
	UUSB_GameManager();
protected:
	LevelManager* m_LevelManager;
	SteamManager* m_SteamManager;
	ConnectablePawnManager* m_ConnectPawnManager;
public:
	void GameOver();

	virtual void Init() override;

	virtual void Shutdown() override;

	void LoadLevel(FName newLevelName);

	void UnloadCurrent();

	void UnloadLevel(FName wantLevel);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Manager")
	UDataTable* m_PawnDataTable;

	

public:
	const FConnectablePawn_Data & GetConnectPawnData(FName id);
};
