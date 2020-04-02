#include "PhysicsPathFollowingComponent.h"

#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

#include "Components/SphereComponent.h"
#include "Engine/EngineBaseTypes.h"
#include "Components/PhysicsSkMeshComponent.h"
#include "UObject/Package.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "AISystem.h"
#include "BrainComponent.h"
#include "Engine/Canvas.h"
#include "AIController.h"
#include "AbstractNavData.h"
#include "NavLinkCustomInterface.h"
#include "Navigation/MetaNavMeshPath.h"
#include "AIConfig.h"

#include "VisualLogger/VisualLoggerTypes.h"
#include "VisualLogger/VisualLogger.h"
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
	const FVector CurrentLocation = MovementComp->GetActorFeetLocation();
	const FVector CurrentTarget = GetCurrentTargetLocation();
	DrawDebugLine(GetWorld(), CurrentLocation, CurrentTarget, FColor::Cyan, false, -1.f, 1, 3.f);
	Super::FollowPathSegment(DeltaTime);


}

	//Choose Which Nav Data To Use
	
//~~~
bool UPhysicsPathFollowingComponent::NavPoly_GetVerts(const NavNodeRef& PolyID, TArray<FVector>& OutVerts)
{
	//Get Nav Data
	const ANavigationData* NavData = JoyGetNavData();

	const ARecastNavMesh* NavMesh = Cast<ARecastNavMesh>(NavData);
	if (!NavMesh)
	{
		return false;
	}

	return NavMesh->GetPolyVerts(PolyID, OutVerts);
}

//Get Box of individual poly from verts
FBox UPhysicsPathFollowingComponent::NavPoly_GetBounds(const NavNodeRef& PolyID)
{
	TArray<FVector> Verts;
	NavPoly_GetVerts(PolyID, Verts);

	FBox Bounds(0);
	for (const FVector& Each : Verts)
	{
		Bounds += Verts;
	}

	return Bounds;
}


////Nav Data Main
//FORCEINLINE const ANavigationData* GetMainNavData(FNavigationSystem::ECreateIfEmpty CreateNewIfNoneFound)
//{
//	UNavigationSystem* NavSys = GetWorld()->GetNavigationSystem();
//	if (!NavSys) return NULL;
//	return NavSys->GetMainNavData(CreateNewIfNoneFound);
//}
//
////Choose Which Nav Data To Use
//FORCEINLINE const ANavigationData* JoyGetNavData() const
//{
//	const FNavAgentProperties& AgentProperties = MovementComp->GetNavAgentPropertiesRef();
//	const ANavigationData* NavData = GetNavDataForProps(AgentProperties);
//	if (NavData == NULL)
//	{
//		VSCREENMSG("ERROR USING MAIN NAV DATA");
//		NavData = GetMainNavData();
//	}
//
//	return NavData;
//}
//
////VERY IMPORTANT FOR CRASH PROTECTION !!!!!
//FORCEINLINE bool TileIsValid(const ARecastNavMesh* NavMesh, int32 TileIndex) const
//{
//	if (!NavMesh) return false;
//	//~~~~~~~~~~~~~~
//	const FBox TileBounds = NavMesh->GetNavMeshTileBounds(TileIndex);
//
//	return TileBounds.IsValid != 0;
//}
//
////add this to your custom path follow component!
//bool NavPoly_GetAllPolys(TArray<NavNodeRef>& Polys);
////Rama's UE4 Nav code to get all the nav polys!
//bool UJoyPathFollowComp::NavPoly_GetAllPolys(TArray<NavNodeRef>& Polys)
//{
//	if (!MovementComp) return false;
//	//~~~~~~~~~~~~~~~~~~
//
//	//Get Nav Data
//	const ANavigationData* NavData = JoyGetNavData();
//
//	const ARecastNavMesh* NavMesh = Cast<ARecastNavMesh>(NavData);
//	if (!NavMesh)
//	{
//		return false;
//	}
//
//	TArray<FNavPoly> EachPolys;
//	for (int32 v = 0; v < NavMesh->GetNavMeshTilesCount(); v++)
//	{
//
//		//CHECK IS VALID FIRST OR WILL CRASH!!! 
//	   //     256 entries but only few are valid!
//		// use continue in case the valid polys are not stored sequentially
//		if (!TileIsValid(NavMesh, v))
//		{
//			continue;
//		}
//
//		NavMesh->GetPolysInTile(v, EachPolys);
//	}
//
//
//	//Add them all!
//	for (int32 v = 0; v < EachPolys.Num(); v++)
//	{
//		Polys.Add(EachPolys[v].Ref);
//	}
//}