// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PolarityObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "PhysicsPolarityObject.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API APhysicsPolarityObject : public APolarityObject
{
	GENERATED_BODY()


public:
	APhysicsPolarityObject();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	/**
	 * 물리 시뮬레이션을 진행할 매쉬이다.
	 * 생성자에서 InitMesh()를 통해 PolarityMesh에 초기화된다.
	 * 당장은 SkeletalMesh는 고려하지 않고 구현하였음
	 */
	UPROPERTY(Category = "Polarity", VisibleAnywhere, BlueprintReadOnly, meta = (DisplayName = "StaticMesh", AllowPrivateAccess = "true"))
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(Category = "Polarity", VisibleAnywhere, BlueprintReadOnly, meta = (DisplayName = "SphereArea", AllowPrivateAccess = "true"))
	USphereComponent* SphereArea;

	
	/** 피직스로 줄 힘의 크기. 기본값은 800 */
	UPROPERTY(Category = "Polarity", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float PhysicsPower;

public:
	virtual void ApplyAffectedForce_Implementation(float DeltaTime);

#if WITH_EDITOR
public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
