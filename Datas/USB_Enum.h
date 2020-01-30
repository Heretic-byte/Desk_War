// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "USB_Enum.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API UUSB_Enum : public UObject
{
	GENERATED_BODY()
	
};

UENUM(BlueprintType)
enum class  EUSB_HeadType : uint8
{
	Basic,
	FourPin,
	OldCom,
	ArduinoSoft,
	ArduinoPower,
	_Length
};


UENUM(BlueprintType)
enum class EUSB_CameraViewType : uint8
{
	Default,
	Bottleneck,
	LevelCamera,
	LookAtUsb,
	_Length
};

UENUM(BlueprintType)
enum class EUSB_PortTracePoint : uint8
{
	Center,
	Left,
	Right,
	_Length
};


