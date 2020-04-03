

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "Components/PhysicsMovement.h"
#include "Camera/CameraComponent.h"
#include "Components/USB_SpringArm.h"
//
#include "PlayerPawn.generated.h"


//USB�� ��Ʈ������ �̻��ϱ⶧���� ��� ȣȯ�ɼ� ����.
//�ٸ� ������ ������Ʈ���� �Ȱ���
//���,�׺��̵��� �ߵǴ��� �׽�Ʈ�ϴ� ����
UCLASS()
class DESK_WAR_API APlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//
protected://component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UPhysicsSkMeshComponent* m_SkMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UPhysicsMovement* m_UsbMovement;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USceneComponent* m_CamRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* m_MainCam;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UUSB_SpringArm* m_MainSpringArm;
	UPROPERTY(VisibleAnywhere, Category = "USB_Body")
	APlayerController* m_PlayerCon;
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	void ZoomIn();
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	void ZoomOut();
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	void Jump();
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	void StopJumping();
	void MoveForward(float v);
	void MoveRight(float v);
	void RotateYaw(float v);
	void RotatePitch(float v);
};
