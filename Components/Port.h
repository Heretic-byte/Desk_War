// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Port.generated.h"

class USkeletalMeshComponent;
class UConnector;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DESK_WAR_API UPort : public USceneComponent
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectBP, UConnector*,connector);
	
public:	
	// Sets default values for this component's properties
	UPort();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:

	// Called every frame
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void SetCollider(UShapeComponent* coll);
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void SetPrmitiveMesh(UPrimitiveComponent* primi);
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void SetConstraint(UPhysicsConstraintComponent* constraint);
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void Connect(UConnector* connector);
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void Disconnect();

	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FOnConnectBP m_OnConnected;
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FOnConnectBP m_OnDisconnected;

	UFUNCTION(BlueprintCallable, Category = "Interact")
	void DisableCollider();
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void EnableCollider();
	UFUNCTION(BlueprintCallable, Category = "Interact")
	bool IsConnected();
	UFUNCTION(BlueprintCallable, Category = "Overlap")
	void OverlapBlock(USceneComponent* collider, USceneComponent* portMesh);
protected:
	UPROPERTY(VisibleAnywhere)
	UConnector* m_ConnectedConnector;
	UPROPERTY(VisibleAnywhere)
	UShapeComponent* m_CollConnector;
	UPROPERTY(VisibleAnywhere)
	UPrimitiveComponent* m_PrmiMeshConnector;
	UPROPERTY(VisibleAnywhere)
	UPhysicsConstraintComponent* m_Constraint;
private:
	void BindConstraintConnector(USkeletalMeshComponent* connectorMesh);
};
