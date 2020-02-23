// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/PinSkMeshComponent.h"
#include "PortSkMeshComponent.generated.h"



//class UPinSkMeshComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DESK_WAR_API UPortSkMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()
public:
	UPortSkMeshComponent(const FObjectInitializer& objInit);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	FName m_NameWantMovePoint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	E_PinPortType m_PortType;
	UPROPERTY(VisibleAnywhere)
	UPinSkMeshComponent* m_ConnectedPin;
	UPROPERTY()
	UPhysicsConstraintComponent* m_ParentPhysicsConst;
	UPROPERTY()
	USkeletalMeshComponent* m_MeshParentActor;
protected:
	FName m_NameParentBonePortPoint;
private:
	void ConstraintPinPort();
	void AdjustPinActorTransform();
	void DisableCollider();
	void EnableCollider();
public:
	UFUNCTION(BlueprintCallable, Category = "Connect Init")
	void InitPort(UPhysicsConstraintComponent* physicsJoint,USkeletalMeshComponent* parentMesh,E_PinPortType portType = E_PinPortType::ENoneType,FName namePinBone = NAME_None);
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void Connect(UPinSkMeshComponent* connector);
	UFUNCTION(BlueprintCallable, Category = "Interact")
	bool Disconnect();
	UFUNCTION(BlueprintCallable, Category = "Interact")
	bool IsConnected();
	UFUNCTION(BlueprintCallable, Category = "Interact")
	E_PinPortType GetPortType() const;
protected:
	virtual void BeginPlay() override;
public:
	FName GetMovePointWant();
	USkeletalMeshComponent* GetParentSkMesh();

	FORCEINLINE E_PinPortType _inline_GetPortType() const
	{
		return m_PortType;
	}
};
