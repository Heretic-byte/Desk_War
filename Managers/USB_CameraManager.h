// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "USB_CameraManager.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API AUSB_CameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
public:
	AUSB_CameraManager();
};
