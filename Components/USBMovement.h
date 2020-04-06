

#pragma once

#include "CoreMinimal.h"
#include "Components/PhysicsMovement.h"

#include "USBMovement.generated.h"



class AUSB_PlayerPawn;
UCLASS()
class DESK_WAR_API UUSBMovement : public UPhysicsMovement
{
	GENERATED_BODY()
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
public:
	virtual void PhysSceneStep(FPhysScene* PhysScene, float DeltaTime) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SetUSBUpdateComponent(AUSB_PlayerPawn* playerPawn,UPhysicsSkMeshComponent* head, UPhysicsSkMeshComponent* tail);
	virtual void AddForce(FVector forceWant) override;
	virtual void AddImpulse(FVector impulseWant) override;
	virtual void BeginPlay() override;
	void RequestConnectChargeMove(const FVector& normalHorizon,float timeWant);
	void RequestAirConnectChargeMove(FRotator portRot,const FVector& normalHorizon, float timeWant);
	void StopUSBMove();
protected:
	virtual bool DoJump() override;
};
