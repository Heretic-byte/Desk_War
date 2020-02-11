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
	FHitResult m_GroundHitResult;
	FName m_NameLinearVelocityBone;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement")
	float m_fGroundCastOffset;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement")
	float m_fMovingForce;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement")
	float m_fJumpZVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement_Rotate")
	FRotator m_RotationRate;
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
	void AddForce(FVector forceWant);
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	void AddImpulse(FVector impulseWant);
private:
	bool IsWalkable(const FHitResult& Hit) const;
	virtual void UpdateComponentVelocity() override;
	FVector ScaleInputAccel(const FVector inputPure) const;
	float GetAxisDeltaRotation(float InAxisRotationRate, float DeltaTime) const;
	FRotator GetDeltaRotation(float DeltaTime) const;
	FRotator ComputeOrientToMovementRotation(const FRotator& CurrentRotation, FRotator& DeltaRotation) const;
	bool DoJump();
private:
	void TickCastGround();
	void TickRotate(float delta);
	void TickMovement(float delta);
public:
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	void SetVelocityBone(FName boneName);
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	void SetMovingForce(float fForce);
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	void SetJumpZVelocity(float zVelo);
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	void SetAngularDamping(float fAngDamp);
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	void SetLinearDamping(float fLinDamp);

};
