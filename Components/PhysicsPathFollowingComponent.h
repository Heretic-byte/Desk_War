

#pragma once

#include "CoreMinimal.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/NavigationSystemBase.h"
#include "NavigationSystem.h"
#include "NavArea.h"
#include "NavigationData.h"
#include "NavMesh/RecastNavMesh.h"

#include "PhysicsPathFollowingComponent.generated.h"

UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class DESK_WAR_API UPhysicsPathFollowingComponent : public UPathFollowingComponent
{
	GENERATED_BODY()
	UPhysicsPathFollowingComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
protected:
	virtual void SetMoveSegment(int32 SegmentStartIndex)override ;
	virtual void FollowPathSegment(float DeltaTime) override;
	virtual void UpdatePathSegment()  override;

};
