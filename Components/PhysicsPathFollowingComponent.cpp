#include "PhysicsPathFollowingComponent.h"

#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

#include "Datas/USB_Macros.h"
#include "DrawDebugHelpers.h"
#include "UnrealMathUtility.h"



//static bool LineBoxIntersection(const FBox& Box, const FVector& Start, const FVector& End, const FVector& Direction);
//
///** Determines whether a line intersects a box. This overload avoids the need to do the reciprocal every time. */
//static bool LineBoxIntersection(const FBox& Box, const FVector& Start, const FVector& End, const FVector& Direction, const FVector& OneOverDirection);
//
///* Swept-Box vs Box test */
//static CORE_API bool LineExtentBoxIntersection(const FBox& inBox, const FVector& Start, const FVector& End, const FVector& Extent, FVector& HitLocation, FVector& HitNormal, float& HitTime);

UPhysicsPathFollowingComponent::UPhysicsPathFollowingComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

}

void UPhysicsPathFollowingComponent::SetMoveSegment(int32 SegmentStartIndex)
{
	Super::SetMoveSegment(SegmentStartIndex);
}

void UPhysicsPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
	Super::FollowPathSegment(DeltaTime);
}

void UPhysicsPathFollowingComponent::UpdatePathSegment()
{
	Super::UpdatePathSegment();
}

