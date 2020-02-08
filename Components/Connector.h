// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Connector.generated.h"


class UPort;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DESK_WAR_API UConnector : public USceneComponent
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectPortOwner, UObject*, portOwner);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectPort, UPort*, port);
public:	
	UConnector(const FObjectInitializer& objInit);
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Body_Mesh")
	USkeletalMeshComponent* m_MeshPin;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Body_Bones")
	FName m_NameNeckBone;
	UPROPERTY(EditDefaultsOnly,  Category = "Body_Bones")
	FName m_NameConnectSocket;
	UPROPERTY(EditDefaultsOnly,  Category = "Body_Bones")
	FName m_NameConnectStartSocket;
	UPROPERTY(EditDefaultsOnly,  Category = "Body_Bones")
	FName m_NameConnectPushPointSocket;
protected:
	UPROPERTY()
	UPort* m_PortConnected;
	UPROPERTY()
	UObject* m_PortOwner;

public:
	FOnConnectPortOwner m_OnConnectedPortOwner;
	FOnConnectPort m_OnConnectedPort;
public:
	FVector GetNeckLoc() const;
	void SetSimulatePhysics(bool v);
	void Connect(UPort* port);
private:
	void CreateHeadMesh();
public:
	FORCEINLINE FName GetBoneNeck()
	{
		return m_NameNeckBone;
	}
	FORCEINLINE USkeletalMeshComponent* GetPin()
	{
		return m_MeshPin;
	}
	FORCEINLINE UPort* GetPortConnected()
	{
		return m_PortConnected;
	}
	
};
