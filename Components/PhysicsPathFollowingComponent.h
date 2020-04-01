

#pragma once

#include "CoreMinimal.h"
#include "Navigation/PathFollowingComponent.h"


#include "PhysicsPathFollowingComponent.generated.h"

UCLASS()
class DESK_WAR_API UPhysicsPathFollowingComponent : public UPathFollowingComponent
{
	GENERATED_BODY()
	UPhysicsPathFollowingComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
protected:
	virtual void SetMoveSegment(int32 SegmentStartIndex)override ;
	virtual void FollowPathSegment(float DeltaTime) override;
	virtual void UpdatePathSegment()  override;

	bool NavPoly_GetVerts(const NavNodeRef& PolyID, TArray<FVector>& OutVerts);

	//Bounds
	FBox NavPoly_GetBounds(const NavNodeRef& PolyID);

};
