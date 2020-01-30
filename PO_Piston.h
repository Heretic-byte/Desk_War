// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PO_MoveAtoB.h"
#include "PO_Piston.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API APO_Piston : public APO_MoveAtoB
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	APO_Piston();

public:
	virtual void Tick(float DeltaTime) override;

public:
	/** 제자리로 돌아오는 속도 */
	UPROPERTY(Category = "Polarity|MoveAtoB|Piston", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float RecoverySpeed;

public:
	UFUNCTION(BlueprintCallable, Category = "Polarity|MoveAtoB|Piston")
	void MoveMeshToHome(float DeltaTime);

};
