// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "USB_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API AUSB_PlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AUSB_PlayerController();
public:
	virtual void BeginPlay() override;
};
