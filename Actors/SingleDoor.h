

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FuncLib/USBFunctionLib.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PuzzleDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CamTimer.h"
#include "EasyActionPlugin/Public/ActionManagerComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "SingleDoor.generated.h"



UCLASS()
class DESK_WAR_API ASingleDoor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASingleDoor();

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door")
	float m_fOpenTime;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door")
	TArray<UMaterialInstanceDynamic*> m_AryMatBrightness;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door")
	TArray<UMaterialInstanceDynamic*> m_AryMatGauge;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Door")
	USkeletalMeshComponent* m_MeshDoor;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door")
	FRotator m_DoorOpenRot;
	UPROPERTY()
	FRotator m_InitDoorRot;

protected:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Door")
	float m_fCamShowTime;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Door")
	UPuzzleDoor* m_PuzzleDoor;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Door")
	UCamTimer* m_CamTimer;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Door")
	UActionManagerComponent* m_ActionManager;

protected:
	UPROPERTY()
	bool m_bDidShow;

protected:
	virtual void BeginPlay() override;

	virtual void OpenDoor();

	virtual void CloseDoor();

	void SetMaterialAry(USkeletalMeshComponent* meshDoor, TArray<UMaterialInstanceDynamic*>& brgtMatAry, TArray<UMaterialInstanceDynamic*>& gagMatAry);

public:
	UFUNCTION(BlueprintCallable, Category = "Door")
	void SetGauge(int matIndexFromAry,float gaugePerOne);

	UFUNCTION(BlueprintCallable, Category = "Door")
	void SetBrightness(int matIndexFromAry, float perOne);

};
//졷까자
//상속가능하게
//문열릴때 콜리전 카메라 없애기
//닫힐때콜리전 카메라 생기기