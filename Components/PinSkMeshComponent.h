// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PhysicsSkMeshComponent.h"
#include "PinSkMeshComponent.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class E_PinPortType :uint8
{
	ENoneType,
	EUSB,
	E5Pin,
	EHDMI,
	ELength
};

class UPortSkMeshComponent;
UCLASS(ClassGroup = (Custom), hidecategories = Object, config = Engine, editinlinenew, meta = (BlueprintSpawnableComponent),BlueprintType, Blueprintable)
class DESK_WAR_API UPinSkMeshComponent : public UPhysicsSkMeshComponent
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectPortOwner, UObject*, portOwner);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectPort, UPortSkMeshComponent*, port);

public:
	UPinSkMeshComponent(const FObjectInitializer& objInit);
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Body_Bones")
	FName m_NameNeckBone;
	UPROPERTY(EditDefaultsOnly, Category = "Body_Bones")
	FName m_NamePinBone;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Body_Bones")
	FName m_NameVelocityPivotBone;
	UPROPERTY(EditDefaultsOnly, Category = "PinType")
	E_PinPortType m_Pintype;
protected:
	UPROPERTY()
	UPortSkMeshComponent* m_PortConnected;
	UPROPERTY()
	UObject* m_PortOwner;
private:
	bool m_AryTypeMatch[(int)E_PinPortType::ELength];
public:
	FOnConnectPortOwner m_OnConnectedPortOwner;
	FOnConnectPort m_OnConnectedPort;
public:
	void SetPinType(E_PinPortType pinType);
	FVector GetNeckLoc() const;
	virtual bool Connect(UPortSkMeshComponent* port);
	virtual bool Disconnect();
	virtual bool CheckTypeMatch(E_PinPortType portsType);
	void SetNeckName(FName nameWant);
	void SetVelocityPivotName(FName nameWant);
private:
	
	virtual void BeginPlay() override;
public:
	FORCEINLINE FName GetNameConnectPoint()
	{
		return m_NamePinBone;
	}
	FORCEINLINE FName GetBoneNeck()
	{
		return m_NameNeckBone;
	}
	FORCEINLINE FName GetBoneVelo()
	{
		return m_NameVelocityPivotBone;
	}
	FORCEINLINE UPortSkMeshComponent* GetPortConnected()
	{
		return m_PortConnected;
	}

};
