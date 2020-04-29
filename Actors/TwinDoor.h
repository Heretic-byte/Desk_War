

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PuzzleDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CamTimer.h"
#include "EasyActionPlugin/Public/ActionManagerComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TwinDoor.generated.h"

UCLASS()
class DESK_WAR_API ATwinDoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATwinDoor();
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door")
	int m_nMatIndex;
	UPROPERTY(BlueprintReadWrite,VisibleAnywhere,Category="Door")
	USkeletalMeshComponent* m_MeshLeftDoor;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Door")
	USkeletalMeshComponent* m_MeshRightDoor;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Door")
	UStaticMeshComponent* m_MeshPropWire;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Door")
	UPuzzleDoor* m_PuzzleDoor;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Door")
	UCamTimer* m_CamTimer;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Door")
	UActionManagerComponent* m_ActionManager;
protected:
	UPROPERTY()
	UMaterialInstanceDynamic* m_Mat;
	UPROPERTY()
	FRotator m_InitLeftRot;
	UPROPERTY()
	FRotator m_InitRightRot;
	UPROPERTY()
	bool m_bDidShow;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OpenDoor();
	void CloseDoor();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable,Category="Door")
	void SetGauge(float gaugePerOne);
};
