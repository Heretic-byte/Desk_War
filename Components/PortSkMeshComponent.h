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
	FName m_NamePinConnectBone;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	E_PinPortType m_PortType;
	UPROPERTY(VisibleAnywhere)
	UPinSkMeshComponent* m_ConnectedPin;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Connect")
	UPhysicsConstraintComponent* m_PhysicsConst;
private:
	bool m_bIsConnected;
private:
	void CreatePhysicsConst();
public:
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void Connect(UPinSkMeshComponent* connector);
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void Disconnect();
	UFUNCTION(BlueprintCallable, Category = "Interact")
	bool IsConnected();
	UFUNCTION(BlueprintCallable, Category = "Interact")
	E_PinPortType GetPortType() const;
protected:
	virtual void BeginPlay() override;
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	
	void DisableCollider();
	void EnableCollider();
	void BindConstraintConnector(USkeletalMeshComponent* connectorMesh);
public:
	FORCEINLINE E_PinPortType _inline_GetPortType() const
	{
		return m_PortType;
	}
};
