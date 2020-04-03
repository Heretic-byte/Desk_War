

#pragma once

#include "CoreMinimal.h"
#include "Components/PhysicsMovement.h"
#include "USBMovement.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API UUSBMovement : public UPhysicsMovement
{
	GENERATED_BODY()
protected:
	UPROPERTY()
	UPhysicsSkMeshComponent* m_MovingTargetTail;
public: 
	void SetUSBUpdateComponent(UPhysicsSkMeshComponent* head, UPhysicsSkMeshComponent* tail);
	virtual void AddForce(FVector forceWant) override;
	virtual void AddImpulse(FVector impulseWant) override;
	virtual void BeginPlay() override;
protected:
	virtual bool DoJump() override;
};
