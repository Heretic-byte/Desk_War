

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Datas/USB_Enum.h"
#include "UObjects/PoolingObj.h"
#include "UObjects/Connectable.h"
#include "ConnectablePawn.generated.h"



UCLASS()
class DESK_WAR_API AConnectablePawn : public APawn,public IConnectable
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AConnectablePawn();

protected:
	EPinPortType m_PinType;
	EPinPortType m_PortType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ConnectPawn")
	FName m_PawnID;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ConnectPawn")
	FText m_PawnName;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ConnectPawn")
	UPhysicsConstraintComponent* m_PhysicsCons;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ConnectPawn")
	USkeletalMeshComponent* m_MeshMainBody;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ConnectPawn")
	USkeletalMeshComponent* m_MeshPort;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ConnectPawn")
	USphereComponent* m_Sphere;//for check player dist
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ConnectPawn")
	UAudioComponent* m_Audio;

protected:
	UPROPERTY()
	UConnectionBehavior* m_ConnectionBehav;
	UPROPERTY()
	FVector m_StartLocation;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void OnConnected(IConnectable* portTarget)override;
	virtual void OnDisconnected(IConnectable* pinTarget)override;

	void SetConnectPawn(FName pawnID);
};
