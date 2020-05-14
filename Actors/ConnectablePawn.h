

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
#include "Navigation/PathFollowingComponent.h"
#include "ConnectablePawn.generated.h"

//
class AAIController;
class UNavigationSystemV1;
class UConnectionBehavior;
//
class UIdleBehavior;
class UReturnBehavior;
class USawPlayerBehavior;
//
class AUSB_PlayerPawn;

class UPhysicsMovement;
class UPawnSensingComponent;
//enum EPathFollowingRequestResult::Type;

UCLASS()
class DESK_WAR_API AConnectablePawn : public APawn,public IConnectable
{
	GENERATED_BODY()

	
public:
	enum class EFSM
	{
		Idle,
		Detect,//player found
		Return,
		Length
	};

	typedef void (AConnectablePawn::*FPtrState)(void);

public:
	AConnectablePawn();

protected:
	float m_fDeltaTime;

	bool m_bUseFSM;

	FPtrState m_AryStateFunction[(int)EFSM::Length];

	EFSM m_CurrentState;

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
	AAIController* m_AiController;
	
	UPROPERTY()
	UConnectionBehavior* m_ConnectionBehav;
	UPROPERTY()
	UIdleBehavior* m_IdleBehavior;
	UPROPERTY()
	USawPlayerBehavior* m_DetectBehavior;
	UPROPERTY()
	UReturnBehavior* m_ReturnToPosBehavior;
	
	UPROPERTY()
	UPhysicsMovement* m_Movement;
	UPROPERTY()
	UPawnSensingComponent* m_PawnSensing;
public:
	UPROPERTY()
	APawn* m_FoundPlayerPawn;
	UPROPERTY()
	FVector m_StartLocation;

private:
	void SetUpSceneComponent(USceneComponent * compo, USceneComponent* parent, FTransform trans);

	void SetUpActorComponent(UActorComponent * compo);

protected:
	void CountingTimer();

	void ExecuteFSM();

	void OnIdle();

	void OnDetectPlayer();

	void OnReturn();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	AUSB_PlayerPawn* GetPlayer() const;
public:
	UFUNCTION()
	void OnSeePlayer(APawn* player);

	AAIController* GetAICon();

	UNavigationSystemV1* GetNav();

	virtual void OnConnected(IConnectable* portTarget)override;

	virtual void OnDisconnected(IConnectable* pinTarget)override;

	void SetConnectPawn(FName pawnID);

	EPathFollowingRequestResult::Type MoveToLocation(FVector loc);

	EPathFollowingRequestResult::Type MoveToActor(AActor* target);

	float GetRadius();

	void SetFSM(EFSM fsm);

public:

	template <typename component>
	FORCEINLINE component* AddSceneComponent(TSubclassOf<component> templateWant, USceneComponent* parent, FTransform trans)
	{
		auto* Created = NewObject<component>(this, templateWant, NAME_None, RF_NoFlags, templateWant->GetDefaultObject<component>());
		Created->SetMobility(EComponentMobility::Movable);
		SetUpSceneComponent(Created, parent, trans);

		return Created;
	}
	template <typename component>
	FORCEINLINE component* AddActorComponent(TSubclassOf<component> templateWant)
	{
		auto* Created = NewObject<component>(this,templateWant,NAME_None,RF_NoFlags, templateWant->GetDefaultObject<component>());

		SetUpActorComponent(Created);

		return Created;
	}

};
