// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"


#include "Datas/USB_Macros.h"

#include "USB_GameManager.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API UUSB_GameManager : public UGameInstance
{
	GENERATED_BODY()
	
public:
	void GameOver();
};
