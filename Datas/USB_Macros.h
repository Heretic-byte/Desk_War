// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "USB_Macros.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API UUSB_Macros : public UObject
{
	GENERATED_BODY()
	
};



//DECLARE_LOG_CATEGORY_EXTERN(LogTemp, Log, All);

#define PRINTF(Format, ...) UE_LOG(LogTemp, Warning, TEXT("%s"), *FString::Printf(TEXT(Format), ##__VA_ARGS__))

#define TEST_BIT(Bitmask, Bit) (((Bitmask) & (static_cast<uint32>(Bit)))>0)
#define SET_BIT(Bitmask, Bit) (Bitmask |= static_cast<uint32>(Bit))
#define CLEAR_BIT(Bitmask, Bit) (Bitmask &= ~( static_cast<uint32>(Bit)))
#define TOFLAG(Enum) ( static_cast<uint32>(Enum))


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FItemDelegate,  UStoredItem*, item, FCoord, gridPivot);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEquipDelegate, UStoredEquipItem*,item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVoidVoidBP);


DECLARE_MULTICAST_DELEGATE(FVoidVoid);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoidFloatBP, float, _floatParam);
DECLARE_MULTICAST_DELEGATE_OneParam(FVoidFloat, float);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoidIntBP, int, _intParam);
DECLARE_MULTICAST_DELEGATE_OneParam(FVoidInt, int);
