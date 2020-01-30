// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PolarityObject.h"
#include "Components/SceneComponent.h"
#include "Components/BillboardComponent.h"
#include "PO_MoveAtoB.generated.h"

/**
 * Interacting시 A와 B 사이로만 움직이는 오브젝트이다.
 */
UCLASS()
class DESK_WAR_API APO_MoveAtoB : public APolarityObject
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APO_MoveAtoB();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	float Alpha;
	float MaxAlpha;
	bool bToDestination;

public:
	UPROPERTY(Category = "Polarity|MoveAtoB", EditAnywhere, BlueprintReadWrite)
	uint8 bCollisionCheck : 1;

	UPROPERTY(Category = "Polarity|MoveAtoB", VisibleInstanceOnly, BlueprintReadWrite)
	USceneComponent* Home;

	UPROPERTY(Category = "Polarity|MoveAtoB", VisibleInstanceOnly, BlueprintReadWrite)
	USceneComponent* Destination;

	UPROPERTY(Category = "Polarity|MoveAtoB", VisibleDefaultsOnly, BlueprintReadWrite)
	FVector Direction;

public:
	UFUNCTION(BlueprintCallable, Category = "Polarity|MoveAtoB")
	void CalcDirection();

	UFUNCTION(BlueprintCallable, Category = "Polarity|MoveAtoB")
	void CalcMaxAlpha();

	// 재귀호출이 가능한 이동함수
	// 힘에 따라 Home~Destination 사이를 움직인다.
	UFUNCTION(BlueprintCallable, Category = "Polarity|MoveAtoB")
	void MoveMesh(FVector NetForce, float DeltaTime, int32 Iterations = 1);

	UFUNCTION(BlueprintCallable, Category = "Polarity|MoveAtoB")
	bool IsForceToDestination(FVector Force);

	// 충돌처리 함수(가상)
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Polarity|MoveAtoB")
	bool CheckCollision();
	virtual bool CheckCollision_Implementation();

	virtual void ApplyAffectedForce_Implementation(float DeltaTime);


// 에디터에서 Home, Destination에 Sprite를 달아주기 위함
#if WITH_EDITORONLY_DATA
private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Display, meta = (AllowPrivateAccess = "true"))
	class UBillboardComponent* HomeSpriteComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Display, meta = (AllowPrivateAccess = "true"))
	class UBillboardComponent* DestinationSpriteComponent;
#endif
};
