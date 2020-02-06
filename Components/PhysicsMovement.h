// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "PhysicsMovement.generated.h"


class UPrimitiveComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DESK_WAR_API UPhysicsMovement : public UPawnMovementComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPhysicsMovement();
protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Debug")
	bool m_bShowDebug;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Physics_Move")
	UPrimitiveComponent* m_UpdateComponent;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Physics_Move")
	float m_fMovingForce;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Physics_Move")
	float m_fLimitLinearVelocity;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Physics_Move")
	float m_fLinearDamping;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Physics_Move")
	float m_fAngularDamping;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Physics_Move")
	float m_fTorqueSpeedWeight;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Physics_Move")
	float m_fTorquePitchSpeedWeight;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Physics_Move")
	float m_fAirControlWeight;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Physics_Move")
	float m_fJumpZVelocity;
protected:
	float m_fOrientRotSpeed;
	float m_fVertical;
	float m_fHorizontal;
	float m_fMaxLinearSpeedSqr;
	bool m_bIsGround;
	FVector m_CurrentGroundNormal;
	FVector m_CurrentUpdateTargetForward;
	FVector m_Velocity;
public:
	UFUNCTION(BlueprintCallable, Category = "Physics_Move")
	bool GetIsGround() const;
	UFUNCTION(BlueprintCallable, Category = "Physics_Move" )
	void DoJump();
	UFUNCTION(BlueprintCallable, Category = "Physics_Move")
	FVector GetVelocity() const;
public:
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	virtual void BeginPlay() override;
	void TickForceMove(float delta);
	void TickHeadYawTorque();
	void TickHeadPitchRotate(const FVector& velocity, float deltaTime);
	void TickLimitVelocity();
	void CastGround(FHitResult& hitResult);
	void ShowDebugVector(FVector dir,float lineLength,FColor colorWant = FColor::Red,float width = 4.f) const;
};
