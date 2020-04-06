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
enum class EPinPortType :uint8
{
	ENoneType,
	EUSB,
	E5Pin,
	EHDMI,
	ELength
};
UENUM(BlueprintType)
enum class  EFailConnectionReason : uint8
{
	PinTypeNotMatch,
	RotationNotMatch,
	PortNotFoundCast,
	PortNotFoundTimeEnd,
	_Length
};



