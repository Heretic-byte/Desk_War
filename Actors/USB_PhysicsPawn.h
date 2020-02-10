// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Datas/USB_Macros.h"
#include "Datas/USB_Enum.h"
#include "Kismet/KismetMathLibrary.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "Components/PinSkMeshComponent.h"

#include "USB_PhysicsPawn.generated.h"

UCLASS()
class DESK_WAR_API AUSB_PhysicsPawn : public APawn
{
	GENERATED_BODY()

public:
	AUSB_PhysicsPawn(const FObjectInitializer& objInit);
protected://components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "USB_Body_Mesh")
	UPinSkMeshComponent* m_PinUSB;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "USB_Body_Mesh")
	UPinSkMeshComponent* m_Pin5Pin;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spine_Physics")
	USplineComponent* m_SpineSpline;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spine_Physics")
	UStaticMesh* m_SpineMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spine_Physics")
	UPhysicalMaterial* m_SpineFriction;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<USphereComponent*> m_ArySpineColls;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<USplineMeshComponent*> m_ArySplineMeshCompos;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Spine_Physics")
	float m_fLineExtraSpacing;
	UPROPERTY(EditDefaultsOnly,  Category = "Spine_Physics")
	float m_fLineRadius;
	UPROPERTY(EditDefaultsOnly,  Category = "Spine_Physics")
	float m_fSpineAngularDamping;
	UPROPERTY(EditDefaultsOnly,  Category = "Spine_Physics")
	float m_fSpineLinearDamping;
	UPROPERTY(EditDefaultsOnly,  Category = "Spine_Physics")
	float m_fCollMass;
	UPROPERTY(EditDefaultsOnly, Category = "Spine_Physics")
	float m_fMaxAngularVelocity;
public:	
	virtual void Tick(float DeltaTime) override;
private:
	void CreatePinUSB();
	void CreatePin4Pin();
	void CreateSpline();
	void UpdateSplinePoint();
	void UpdateSplineMesh();
public:
	UFUNCTION(BlueprintCallable, Category = "Init")
	virtual void InitUSB() ;
	UFUNCTION(BlueprintCallable, Category = "Init")
	int SetTailLocation();
	UFUNCTION(BlueprintCallable, Category = "Init")
	void SpawnSpineColls(int nSpineCount);
	UFUNCTION(BlueprintCallable, Category = "Init")
	void InitSplineComponent();
	UFUNCTION(BlueprintCallable, Category = "Init")
	void InitSplineMesh();
	UFUNCTION(BlueprintCallable, Category = "Init")
	void InitPhysicsConstraints();
private:
	void SetUpSceneComponent(USceneComponent * compo, USceneComponent* parent, FTransform trans);
	void SetUpActorComponent(UActorComponent * compo);
public:
	UPhysicsConstraintComponent* AddPhysicsConstraint(const FTransform trans);
	template <typename component>
	FORCEINLINE component* AddSceneComponent(TSubclassOf<component> templateWant, USceneComponent* parent, FTransform trans)
	{
		auto* Created = NewObject<component>(this, NAME_None, RF_NoFlags, templateWant->GetDefaultObject<component>());
		Created->SetMobility(EComponentMobility::Movable);
		SetUpSceneComponent(Created, parent, trans);

		return Created;
	}
	template <typename component>
	FORCEINLINE component* AddActorComponent(TSubclassOf<component> templateWant)
	{
		auto* Created = NewObject<component>(this, NAME_None, RF_NoFlags, templateWant->GetDefaultObject<component>());
		SetUpActorComponent(Created);

		return Created;
	}
};
