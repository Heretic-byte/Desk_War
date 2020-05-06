

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PuzzleKey.h"
#include "FuncLib/USBFunctionLib.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PuzzleDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Multitab.generated.h"

UCLASS()
class DESK_WAR_API AMultitab : public AActor
{
	GENERATED_BODY()
	
public:	
	AMultitab(const FObjectInitializer& objInit);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Puzzle")
	UStaticMeshComponent* m_MeshMultitabBody;
	UPROPERTY(VisibleAnywhere, Category = "Puzzle")
	USkeletalMeshComponent* m_MeshPowerButton;
	UPROPERTY(VisibleAnywhere,Category="Puzzle")
	UPuzzleKey* m_PuzzleKey;
	UPROPERTY(VisibleAnywhere, Category = "Puzzle")
	UAudioComponent* m_AudioComp;
	UPROPERTY(VisibleAnywhere, Category = "Puzzle")
	UBoxComponent* m_CollButtonDown;
	UPROPERTY(VisibleAnywhere, Category = "Puzzle")
	UBoxComponent* m_CollButtonUp;
	UPROPERTY()
	UAnimationAsset* m_OpenAnim;

	UPROPERTY()
	UAnimationAsset* m_CloseAnim;

protected:
	bool m_bButtonOff;

	UMaterialInstanceDynamic* m_MatForSwitch;

protected:
	virtual void BeginPlay() override;

	void OpenSwitch();

	void CloseSwitch();

public:
	UFUNCTION()
	void OverlapOpen(UPrimitiveComponent* overlappedSelf,AActor* otherActor,UPrimitiveComponent* otherComp,int32 bodyIndex,bool fromSweep,const FHitResult& sweepResult);
	UFUNCTION()
	void OverlapClose(UPrimitiveComponent* overlappedSelf, AActor* otherActor, UPrimitiveComponent* otherComp, int32 bodyIndex, bool fromSweep, const FHitResult& sweepResult);

};
//SkeletalMesh'/Game/Meshes/Prop_205/Tutorial/Gate/SK_Multitap_Button.SK_Multitap_Button'