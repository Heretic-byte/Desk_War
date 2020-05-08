// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Components/PortSkMeshComponent.h"
#include "Components/PinSkMeshComponent.h"
#include "Components/SphereComponent.h"
#include "UObjects/IPoolingObj.h"
#include "Actors/ObjectGiver.h"
#include "PortPawn.generated.h"

UCLASS()
class DESK_WAR_API APortPawn : public APawn,public IIPoolingObj
{
	GENERATED_UCLASS_BODY()
public:
	static FName MeshComponentName;
	static FName MeshPortComponentName;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USphereComponent* m_Sphere;//for check player dist
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UPhysicsSkMeshComponent* m_Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category = "Mesh")
	UPortSkMeshComponent* m_MeshPort;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category = "Connect")
	UPhysicsConstraintComponent* m_PhyConPort;
	UPROPERTY()
	AObjectGiver* m_Spawner;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Connect")
	bool m_bIsUsingSpawn;
private:
	void CreateMesh();
	void CreatePort();
	void CreatePhyCon();
	void CreateSphereColl();

protected:
	virtual void PortConnected(UPinSkMeshComponent* pinConnect);
	virtual void PortDisConnected(UPinSkMeshComponent* pinConnect);

protected:
	virtual void BeginPlay() override;

	virtual void InitPortPawn();

public:
	virtual void OnInit(AObjectGiver* objGiver) override;
	virtual void OnPullEnque() override;
};
