// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PhysicsSkMeshComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/PinSkMeshComponent.h"
#include "Datas/USB_Enum.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "PortSkMeshComponent.generated.h"


class USphereComponent;
//class UPinSkMeshComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DESK_WAR_API UPortSkMeshComponent : public UPhysicsSkMeshComponent
{
	GENERATED_BODY()
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FPinConnection, UPinSkMeshComponent*);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPinConnectionBP, UPinSkMeshComponent*,pinSk);
	UPortSkMeshComponent(const FObjectInitializer& objInit);
public:
	UPROPERTY(BlueprintAssignable,Category="Interact")
	FPinConnectionBP m_OnConnectedBP;
	FPinConnection m_OnConnected;
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FPinConnectionBP m_OnDisconnectedBP;
	FPinConnection m_OnDisconnected;
public:
	UPROPERTY(EditDefaultsOnly, Category = "Body_Bones")
	float m_fFailImpulsePower;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	float m_fEjectPowerToPin;
	FName m_NameInitCollProfile;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	bool m_bCantMoveOnConnected;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Blink")
	float m_fBlinkDelay;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blink")
	FLinearColor m_MatPortFailColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blink")
	FLinearColor m_MatPortSuccessColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blink")
	FName m_NameMatScalarParam;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blink")
	FName m_NameMatVectorParam;
	
	UPROPERTY()
	UMaterialInterface* m_BlinkMat;
	UPROPERTY()
	UMaterialInstanceDynamic* m_BlinkMatDynamic;
	UPROPERTY(EditDefaultsOnly, Category = "Connection")
	FRotator m_ConnectableRotation;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	float m_fEjectPowerSelf;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	EPinPortType m_PortType;
	UPROPERTY(VisibleAnywhere)
	UPinSkMeshComponent* m_ConnectedPin;
	UPROPERTY()
	UPhysicsConstraintComponent* m_ParentPhysicsConst;
	UPROPERTY()
	UPhysicsSkMeshComponent* m_MeshParentActor;
	UPROPERTY()
	USphereComponent* m_CollSphere;
protected:
	FLinearColor m_MatInitColor;
	FName m_NameParentBonePortPoint;
	float m_fCurrentBlinkDelay;
	float m_fCurrentBlinkDelayTimer;
	bool m_bIsBlinked;
	bool m_bIsBlinkStart;
	float m_fBlinkInterpCache;
	float m_fMatBrightness;
private:
	void ConstraintPinPort();
public:
	
	UFUNCTION(BlueprintCallable, Category = "Connection")
	virtual void InitPort(UPhysicsConstraintComponent* physicsJoint,
		UPhysicsSkMeshComponent* parentMesh,
		USphereComponent* sphereColl,
		EPinPortType portType = EPinPortType::ENoneType);
	UFUNCTION(BlueprintCallable, Category = "Interact")
	virtual void Connect(UPinSkMeshComponent* connector);
	UFUNCTION(BlueprintCallable, Category = "Interact")
	virtual bool Disconnect();
	UFUNCTION(BlueprintCallable, Category = "Interact")
	bool IsConnected();
	UFUNCTION(BlueprintCallable, Category = "Interact")
	EPinPortType GetPortType() const;
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void OnPlayerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void FailConnection(const FHitResult & hitResult);
protected:
	virtual void BeginPlay() override;
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) override;
	void StartBlink(float blinkDe);
	void EndBlink();
	void SetPortMatScalar(FName paramName, float scalar);
	void SetPortMatColor(FName paramName, FLinearColor color);
	bool CheckConnectTransform(USceneComponent * connector,bool isConnectorGround);
	bool CheckYawOnly(USceneComponent * connector);
	void EnableOverlap();
	void DisableOverlap();
	UPhysicsSkMeshComponent* GetParentSkMesh();
	void OnFocus(UPinSkMeshComponent * aimingPin, bool isConnectorGround);
	void OnFocusEnd(UPinSkMeshComponent * aimingPin);

	FORCEINLINE EPinPortType _inline_GetPortType() const
	{
		return m_PortType;
	}

	FORCEINLINE UPinSkMeshComponent* GetPinConnected()
	{
		return m_ConnectedPin;
	}
};
