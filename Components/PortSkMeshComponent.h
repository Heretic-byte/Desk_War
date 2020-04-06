// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PhysicsSkMeshComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/PinSkMeshComponent.h"
#include "Datas/USB_Enum.h"
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
public:
	UPROPERTY(EditDefaultsOnly, Category = "Body_Bones")
	float m_fFailImpulsePower;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Connection")
	FRotator m_ConnectableRotation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Body")
	FName m_NamePortConnectSocket;//ConnectPoint?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Body")
	FName m_NamePortConnectStartSocket;//삽입을 하기전 첫번째 목적지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Body")
	FName m_NamePortConnectPushPointSocket;//들이박아서 끼워지는 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	bool m_bBlockMoveOnConnected;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	float m_fEjectPower;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	float m_fConnectableDistSqr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	FName m_NameWantMovePoint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	EPinPortType m_PortType;
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
	UFUNCTION(BlueprintCallable, Category = "Connection")
	virtual void InitPort(UPhysicsConstraintComponent* physicsJoint, UPhysicsSkMeshComponent* parentMesh,EPinPortType portType = EPinPortType::ENoneType,FName namePinBone = NAME_None);
	UFUNCTION(BlueprintCallable, Category = "Interact")
	virtual void Connect(UPinSkMeshComponent* connector);
	UFUNCTION(BlueprintCallable, Category = "Interact")
	virtual bool Disconnect();
	UFUNCTION(BlueprintCallable, Category = "Interact")
	bool IsConnected();
	UFUNCTION(BlueprintCallable, Category = "Interact")
	EPinPortType GetPortType() const;

	void FailConnection(const FHitResult & hitResult);
protected:
	virtual void BeginPlay() override;
public:
	bool CheckConnectTransform(USceneComponent * connector,bool isConnectorGround);
	void EnablePhysicsCollision();
	void DisablePhysicsCollision();
	bool GetBlockMoveOnConnnect();
	FName GetMovePointWant();
	UPhysicsSkMeshComponent* GetParentSkMesh();

	FORCEINLINE EPinPortType _inline_GetPortType() const
	{
		return m_PortType;
	}

	FORCEINLINE FVector _inline_GetConnectPoint() const
	{
		return GetSocketLocation(m_NamePortConnectSocket);
	}
	FORCEINLINE FVector _inline_GetConnectReadyPoint() const
	{
		return GetSocketLocation(m_NamePortConnectStartSocket);
	}
	FORCEINLINE FVector _inline_GetPushPoint() const
	{
		return GetSocketLocation(m_NamePortConnectPushPointSocket);
	}

	FORCEINLINE UPinSkMeshComponent* GetPinConnected() const
	{
		return m_ConnectedPin;
	}
};
