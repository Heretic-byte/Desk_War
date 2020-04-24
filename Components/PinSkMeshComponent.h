// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PhysicsSkMeshComponent.h"
#include "Datas/USB_Enum.h"
#include "PinSkMeshComponent.generated.h"

/**
 * 
 */


class UPortSkMeshComponent;
UCLASS(ClassGroup = (Custom), hidecategories = Object, config = Engine, editinlinenew, meta = (BlueprintSpawnableComponent),BlueprintType, Blueprintable)
class DESK_WAR_API UPinSkMeshComponent : public UPhysicsSkMeshComponent
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectPortOwnerBP, UObject*, portOwner);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectPortBP, UPortSkMeshComponent*, port);

public:
	UPinSkMeshComponent(const FObjectInitializer& objInit);
public:
	UPROPERTY(EditDefaultsOnly, Category = "Body_Bones")
	float m_fFailImpulsePower;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Body_Bones")
	FName m_NameNeckBone;
	UPROPERTY(EditDefaultsOnly, Category = "Body_Bones")
	FName m_NamePinBone;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Body_Bones")
	FName m_NameVelocityPivotBone;
	UPROPERTY(EditDefaultsOnly, Category = "PinType")
	EPinPortType m_Pintype;
protected:
	UPROPERTY()
	UPortSkMeshComponent* m_PortConnected;//my pin connected
	UPROPERTY()
	UPortSkMeshComponent* m_MyPort;//my self port
	UPROPERTY()
	UObject* m_PortOwner;
private:
	bool m_AryTypeMatch[(int)EPinPortType::ELength];
public:
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FOnConnectPortOwnerBP m_OnConnectedPortOwnerBP;
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FOnConnectPortBP m_OnConnectedPortBP;
public:
	void SetPinType(EPinPortType pinType);
	FVector GetNeckLoc() const;
	virtual bool Connect(UPortSkMeshComponent* port);
	virtual bool Disconnect();
	virtual bool CheckTypeMatch(EPinPortType portsType);
	void SetNeckName(FName nameWant);
	void SetVelocityPivotName(FName nameWant);
	void FailConnection(const FHitResult & hitResult);
	void SetMyPort(UPortSkMeshComponent* portMine);
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

	FORCEINLINE UPortSkMeshComponent* GetMyPort()//adopters port
	{
		return m_MyPort;
	}
};
