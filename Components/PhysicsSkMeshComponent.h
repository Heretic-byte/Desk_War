

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
public:
	UFUNCTION()
	void OnBlockHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
