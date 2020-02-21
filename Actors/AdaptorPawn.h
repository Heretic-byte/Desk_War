

#pragma once

#include "CoreMinimal.h"
#include "Actors/PortPawn.h"
#include "AdaptorPawn.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API AAdaptorPawn : public APortPawn
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UPinSkMeshComponent* m_MeshPin;
};
