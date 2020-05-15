

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"

#include "Datas/USB_Enum.h"
#include "UObjects/SawPlayerBehavior.h"
#include "UObjects/IdleBehavior.h"
#include "UObjects/ReturnBehavior.h"
#include "UObjects/ConnectionBehavior.h"




#include "ConnectablePawnData.generated.h"

class UPhysicalMaterial;



UCLASS()
class DESK_WAR_API UConnectablePawnData : public UObject
{
	GENERATED_BODY()
	
};

USTRUCT(BlueprintType)
struct FConnectablePawn_Data : public FTableRowBase
{
	GENERATED_BODY()

public:
	FConnectablePawn_Data()
	{
		m_NameID = NAME_None;
		m_bIsAI = false;
		m_IdleBehav = UIdleBehavior::StaticClass();
		m_SawPlayerBehav = USawPlayerBehavior::StaticClass();
		m_ReturnPlayerBehav = UReturnBehavior::StaticClass();
		m_fInteractRadius=250.f;
	}

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName m_NameID;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText m_ShowingName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EPinPortType m_PinType;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EPinPortType m_PortType;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USkeletalMesh* m_MeshPawnMainBody;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float m_fInteractRadius;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USkeletalMesh* m_MeshPortBody;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UPhysicalMaterial* m_FrictionMat;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector m_PortRelativeLoc;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRotator m_PortRelativeRot;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UConnectionBehavior> m_ConnectBehav;//다넣어야함//얘네초기값은 어떻게? strategy
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool m_bIsAI;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float m_fMovingForce = 600.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float m_fMaxSpeed = 370.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float m_fMaxBrakingDeceleration = 240.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int m_nJumpMaxCount = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float m_fJumpHeight = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float m_fHearingThreshold = 900.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float m_fLOSHearingThreshold = 1800.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float m_fSightRadius = 900.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float m_fPawnSensingTick = 0.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float m_fAngle = 80.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UIdleBehavior> m_IdleBehav;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<USawPlayerBehavior> m_SawPlayerBehav;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UReturnBehavior> m_ReturnPlayerBehav;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimationAsset* m_IdleAnim;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimationAsset* m_JumpAnim;
};

//ai
//pawn sensing
//무브먼트
//fsm 로직
//idle
//on player saw