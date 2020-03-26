

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsSkMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API UPhysicsSkMeshComponent : public USkeletalMeshComponent
{
	GENERATED_UCLASS_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	UCapsuleComponent* m_BoundingCapsule;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category="Physics")
	float m_fMeshRadiusMultiple;
public:
	float GetMeshRadiusMultiple();
	void SetMeshRadiusMultiple(float vM);

	UFUNCTION(BlueprintCallable)
	void SetBoundingCapsule(UCapsuleComponent* capsule);
	FORCEINLINE UCapsuleComponent* GetBoundingCapsule()
	{
		return m_BoundingCapsule;
	}
};
