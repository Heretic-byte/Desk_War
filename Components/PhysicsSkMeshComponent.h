

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsSkMeshComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), hidecategories = Object, config = Engine, editinlinenew, meta = (BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class DESK_WAR_API UPhysicsSkMeshComponent : public USkeletalMeshComponent
{
	GENERATED_UCLASS_BODY()
};
