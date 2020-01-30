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
class DESK_WAR_API UConnector : public UActorComponent
{
	GENERATED_BODY()
public:	
	UConnector();
protected:
	virtual void BeginPlay() override;
protected:
	UPROPERTY()
	UCapsuleComponent* m_CollConnector;
	UPROPERTY()
	USkeletalMeshComponent* m_MeshConnector;
	UPROPERTY()
	UPort* m_PortConnected;
	UPROPERTY()
	UObject* m_PortOwner;
public:
	FORCEINLINE UCapsuleComponent* GetColl()
	{
		return m_CollConnector;
	}
	FORCEINLINE void SetColl(UCapsuleComponent* coll)
	{
		m_CollConnector = coll;
	}

	FORCEINLINE USkeletalMeshComponent* GetMesh()
	{
		return m_MeshConnector;
	}
	FORCEINLINE void SetMesh(USkeletalMeshComponent* mesh)
	{
		m_MeshConnector = mesh;
	}

	FORCEINLINE UPort* GetPortConnected()
	{
		return m_PortConnected;
	}
	void Connect(UPort* port);
};
