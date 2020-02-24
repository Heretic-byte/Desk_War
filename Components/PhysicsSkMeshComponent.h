

#pragma once

#include "CoreMinimal.h"
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category="Physics")
	float m_fMeshRadiusMultiple;
public:
	float GetMeshRadiusMultiple();
	void SetMeshRadiusMultiple(float vM);
};
