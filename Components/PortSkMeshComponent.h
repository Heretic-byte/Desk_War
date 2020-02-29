// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PhysicsSkMeshComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/PinSkMeshComponent.h"
#include "PortSkMeshComponent.generated.h"



//class UPinSkMeshComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DESK_WAR_API UPortSkMeshComponent : public UPhysicsSkMeshComponent
{
	GENERATED_BODY()
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FPinConnection, UPinSkMeshComponent*);
	UPortSkMeshComponent(const FObjectInitializer& objInit);
public:
	FPinConnection m_OnConnected;
	FPinConnection m_OnDisconnected;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	bool m_bBlockMoveOnConnected;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	float m_fEjectPower;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	float m_fConnectableDistSqr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	FName m_NameWantMovePoint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	E_PinPortType m_PortType;
	UPROPERTY(VisibleAnywhere)
	UPinSkMeshComponent* m_ConnectedPin;
	UPROPERTY()
	UPhysicsConstraintComponent* m_ParentPhysicsConst;
	UPROPERTY()
	UPhysicsSkMeshComponent* m_MeshParentActor;
protected:
	FName m_NameParentBonePortPoint;
private:
	void ConstraintPinPort();
public:
	UFUNCTION(BlueprintCallable, Category = "Connect Init")
	virtual void InitPort(UPhysicsConstraintComponent* physicsJoint, UPhysicsSkMeshComponent* parentMesh,E_PinPortType portType = E_PinPortType::ENoneType,FName namePinBone = NAME_None);
	UFUNCTION(BlueprintCallable, Category = "Interact")
	virtual void Connect(UPinSkMeshComponent* connector);
	UFUNCTION(BlueprintCallable, Category = "Interact")
	virtual bool Disconnect();
	UFUNCTION(BlueprintCallable, Category = "Interact")
	bool IsConnected();
	UFUNCTION(BlueprintCallable, Category = "Interact")
	E_PinPortType GetPortType() const;
protected:
	virtual void BeginPlay() override;
public:
	void DisablePhysics();
	void EnablePhysics();
	void EnablePhysicsCollision();
	void DisblePhysicsCollision();
	bool SetAimTracePoint(FVector& tracedImpactPoint);
	bool GetBlockMoveOnConnnect();
	FName GetMovePointWant();
	UPhysicsSkMeshComponent* GetParentSkMesh();

	FORCEINLINE E_PinPortType _inline_GetPortType() const
	{
		return m_PortType;
	}
};
