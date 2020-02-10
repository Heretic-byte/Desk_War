// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Datas/USB_Macros.h"
#include "GameFramework/PawnMovementComponent.h"
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
private:
	UPrimitiveComponent* m_MovingTarget;
	USceneComponent* m_DeferredUpdatedMoveComponent;
	bool m_bMovementInProgress;
	bool m_bDeferUpdateMoveComponent;
	FVector m_LastUpdateVelocity;
	FVector m_PendingImpulseToApply;
	FVector m_PendingForceToApply;
	FVector m_PendingLaunchVelocity;
	FVector m_Acceleration;
	bool m_bOnGround;
	bool m_bPressedJump;
	float m_fWalkableFloorZ;
protected:
	UPROPERTY(EditDefaultsOnly,Category="PhysicsMovement")
	FName m_NameLinearVelocityBone;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement")
	float m_fMovingForce;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement")
	float m_fJumpZVelocity;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "PhysicsMovement_Jump")
	bool m_bWasJumping;
	UPROPERTY(Transient, BlueprintReadOnly, VisibleInstanceOnly, Category = "PhysicsMovement_Jump")
	float m_fJumpKeyHoldTime;
	UPROPERTY(Transient, BlueprintReadOnly, VisibleInstanceOnly, Category = "PhysicsMovement_Jump")
	float m_fJumpForceTimeRemaining;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement_Jump", Meta = (ClampMin = 0.0, UIMin = 0.0))
	float m_fJumpMaxHoldTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement_Jump")
	int32 m_nJumpMaxCount;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "PhysicsMovement_Jump")
	int32 m_nJumpCurrentCount;
	UPROPERTY(EditAnywhere, Category = "PhysicsMovement_Slope",meta = (ClampMin = "0.0", ClampMax = "90.0", UIMin = "0.0", UIMax = "90.0"))
	float m_fWalkableFloorAngle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement_Rotate")
	FRotator RotationRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement")
	float m_fAngularDampingForPhysicsAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement")
	float m_fLinearDampingForPhysicsAsset;
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
	void Launch(FVector const& LaunchVel);
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	void AddForce(FVector forceWant);
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	void AddImpulse(FVector impulseWant);
private:
	bool IsWalkable(const FHitResult& Hit) const;
	virtual void UpdateComponentVelocity();
	void ClearAccumulatedForces();
	FVector ScaleInputAccel(const FVector inputPure) const;
	void ApplyAccumulatedForces(float DeltaSeconds);
	void PerformMovement(float delta);
	bool Rotate(float delta);
	float GetAxisDeltaRotation(float InAxisRotationRate, float DeltaTime) const;
	FRotator GetDeltaRotation(float DeltaTime) const;
	FRotator ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation) const;
private://slopeWalkable
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement_Slope")
	void SetWalkableFloorAngle(float InWalkableFloorAngle);
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement_Slope")
	void SetWalkableFloorZ(float InWalkableFloorZ);

	FORCEINLINE float GetWalkableFloorAngle() const
	{
		return m_fWalkableFloorAngle;
	}
	FORCEINLINE float GetWalkableFloorZ() const
	{
		return m_fWalkableFloorZ;
	}
private://jump
	void CheckJumpInput(float delta);
	bool IsJumpProvidingForce() const;
	bool DoJump();
	bool CanJump();
	float GetJumpMaxHoldTime() const;
	void ResetJumpState();
	void ClearJumpInput(float delta);
	bool HandlePendingLaunch();
public:
	virtual bool IsFalling() const override;
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
