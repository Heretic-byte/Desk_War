// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Datas/USB_Macros.h"
#include "GameFramework/PawnMovementComponent.h"
#include "CollisionQueryParams.h"
#include "PhysicsMovement.generated.h"

/**
 * 설계 목적:
 해당 게임에 등장하는 플레이어 및 npc가 
 서로 물리 상호작용 + 네비게이션 이동까지 되야함.
 */




UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class DESK_WAR_API UPhysicsMovement : public UPawnMovementComponent
{
	GENERATED_BODY()
public:
	UPhysicsMovement(const FObjectInitializer& objInit);
public:
	//FName Test;
	//UPrimitiveComponent* TestTail;
private:
	UPROPERTY()
	UPrimitiveComponent* m_MovingTarget;

	TArray<AActor*>* m_ptrAryTraceIgnoreActors;
private:
	FVector m_InputNormal;
	FVector m_Acceleration;
	bool m_bOnGround;
	bool m_bPressedJump;
	FHitResult m_GroundHitResult;
	//jump
	bool m_bWasJumping;
	float m_fJumpKeyHoldTime;
	int m_nJumpCurrentCount;
	float m_fJumpForceTimeRemaining;
	//
public:
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement")
	FName m_NameLinearVelocityBone;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement")
	bool m_bDebugShowForwardCast;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement", meta = (ClampMin = "0", UIMin = "0"))
	float m_fGroundCastBoxSize;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement", meta = (ClampMin = "0", UIMin = "0"))
	float m_fForwardCastOffset;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement", meta = (ClampMax = "0", UIMax = "0"))
	float m_fGroundCastOffset;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement", meta = (ClampMin = "0", UIMin = "0"))
	float m_fMovingForce;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement_Rotate")
	FRotator m_RotationRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement", meta = (ClampMin = "0", UIMin = "0"))
	float m_fAngularDampingForPhysicsAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement", meta = (ClampMin = "0", UIMin = "0"))
	float m_WalkableSlopeAngle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement", meta = (ClampMin = "0", UIMin = "0"))
	float m_WalkableSlopeHeight;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement_Jump", meta = (ClampMin = "0", UIMin = "0"))
	float m_fJumpZVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement", meta = (ClampMin = "0", UIMin = "0"))
	float m_fLinearDampingForPhysicsAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement_Jump", meta = (ClampMin = "0", UIMin = "0"))
	float m_fMaxHoldTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement_Jump", meta = (ClampMin = "0", UIMin = "0"))
	int m_nJumpMaxCount;
	UPROPERTY(Category = "PhysicsMovement_Jump", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float m_fAirControl;
public:
	UPROPERTY(BlueprintAssignable, Category = "PhysicsMovement_Jump")
	FVoidVoidBP m_OnJumpBP;
	FVoidVoid m_OnJump;
public:
	UFUNCTION(BlueprintCallable,Category="PhysicsMovement_Jump")
	void Jump();
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement_Jump")
	void StopJumping();
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	bool IsGround() const;
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	float GetMaxForce() const;
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	void AddForce(FVector forceWant);
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	void AddImpulse(FVector impulseWant);
private:
	virtual void UpdateComponentVelocity() override;
	FVector ScaleInputAccel(const FVector inputPure) ;
	float GetAxisDeltaRotation(float InAxisRotationRate, float DeltaTime) const;
	FRotator GetDeltaRotation(float DeltaTime) const;
	FRotator ComputeOrientToMovementRotation(const FRotator& CurrentRotation, FRotator& DeltaRotation) const;
	void AddIgnoreActorsToQuery(FCollisionQueryParams& queryParam);
protected:
	virtual bool IsFalling()  const override;
	virtual void BeginPlay() override;
private:
	void ResetJumpState();
	void CheckJumpInput(float delta);
	void ClearJumpInput(float delta);
	bool CanJump() const;
	bool DoJump();
private:
	bool TickCheckCanMoveForward();
	void TickCastGround();
	void TickRotate(float delta);
	void TickMovement(float delta);
public:
	void SetTraceIgnoreActorAry(TArray<AActor*>* aryWant);
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SetDamping(float fLinDamp, float fAngDamp);
};
