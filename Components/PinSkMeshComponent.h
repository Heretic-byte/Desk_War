// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
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

class UPort;
UCLASS()
class DESK_WAR_API UPinSkMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()
public:

	

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectPortOwner, UObject*, portOwner);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectPort, UPort*, port);

public:
	UPinSkMeshComponent(const FObjectInitializer& objInit);
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Body_Bones")
	FName m_NameNeckBone;
	UPROPERTY(EditDefaultsOnly, Category = "Body_Bones")
	FName m_NameConnectSocket;
	UPROPERTY(EditDefaultsOnly, Category = "Body_Bones")
	FName m_NameConnectStartSocket;
	UPROPERTY(EditDefaultsOnly, Category = "Body_Bones")
	FName m_NameConnectPushPointSocket;
	UPROPERTY(EditDefaultsOnly, Category = "Body_Bones")
	E_PinPortType m_Pintype;
protected:
	UPROPERTY()
	UPort* m_PortConnected;
	UPROPERTY()
	UObject* m_PortOwner;
private:
	bool m_AryTypeMatch[(int)E_PinPortType::ELength];
public:
	FOnConnectPortOwner m_OnConnectedPortOwner;
	FOnConnectPort m_OnConnectedPort;
public:
	FVector GetNeckLoc() const;
	virtual void Connect(UPort* port);
	virtual bool CheckTypeMatch(E_PinPortType portsType);
	void SetNeckName(FName nameWant);
private:
	virtual void BeginPlay() override;
	void SetTypeMatch();
public:
	FORCEINLINE FName GetBoneNeck()
	{
		return m_NameNeckBone;
	}
	FORCEINLINE UPort* GetPortConnected()
	{
		return m_PortConnected;
	}

};
