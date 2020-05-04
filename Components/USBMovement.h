

#pragma once

#include "CoreMinimal.h"
#include "Components/PhysicsMovement.h"

#include "Datas/USB_Macros.h"
#include "USBMovement.generated.h"



class AUSB_PlayerPawn;
UCLASS()
class DESK_WAR_API UUSBMovement : public UPhysicsMovement
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "PhysicsMovement_Jump")
	FOnLandingBP m_OnTailLandingBP;
	UPROPERTY(BlueprintAssignable, Category = "PhysicsMovement_Jump")
	FVoidVoidBP m_OnJumpBP;
	UPROPERTY(BlueprintAssignable, Category = "PhysicsMovement_Jump")
	FVoidIntBP m_OnJumpSeveralBP;
public:
	UUSBMovement(const FObjectInitializer& objInit);
protected:
	UPROPERTY()
	UPhysicsSkMeshComponent* m_MovingTargetTail;
	UPROPERTY()
	AUSB_PlayerPawn* m_PlayerPawn;
	float m_fAutoMoveTimeWant;
	float m_fAutoMoveTimer;
	FVector m_fAutoMoveInput;
	float m_fInitHeadMass;
	FHitResult m_TailGroundHitResult;
	FCollisionShape m_TailShape;
	float m_fTailGroundDist;
	bool m_bTailOnGround;
	bool m_bTailIsFalling;
	float m_fTailFallStartZ;
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void InitUSBUpdateComponent(AUSB_PlayerPawn* playerPawn,UPhysicsSkMeshComponent* head, UPhysicsSkMeshComponent* tail);
	void SetUSBUpdateComponent(UPhysicsSkMeshComponent * head, UPhysicsSkMeshComponent * tail, bool bRemoveTraceOld);
	virtual void AddForce(FVector forceWant) override;
	virtual void AddImpulse(FVector impulseWant) override;
	virtual void BeginPlay() override;
	void RequestConnectChargeMove(const FVector& normalHorizon,float timeWant,float speedM);
	void RequestAirConnectChargeMove(FRotator portRot,const FVector& normalHorizon, float timeWant,float speedM);
	void StopUSBMove();
	void ClearAirRotation();

protected:
	virtual bool DoJump() override;

	virtual void TickCastGround() override;

	void TailLanding();
	
	virtual void CollectHeight() override;
};
