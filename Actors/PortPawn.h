// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Components/PortSkMeshComponent.h"
#include "Components/PinSkMeshComponent.h"
#include "PortPawn.generated.h"

UCLASS()
class DESK_WAR_API APortPawn : public APawn
{
	GENERATED_UCLASS_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* m_MeshPawn;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category = "Mesh")
	UPortSkMeshComponent* m_MeshPort;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category = "Connect")
	UPhysicsConstraintComponent* m_PhyConPort;

protected:
	virtual void BeginPlay() override;
};
