

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Datas/USB_Macros.h"
#include "PuzzleDoor.generated.h"
class UPuzzleKey;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DESK_WAR_API UPuzzleDoor : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPuzzleDoor();

protected:
	UPROPERTY(EditAnywhere, Category = "Puzzle")
	int m_nDoorUnlockCount;//다열지 않아도 될때
	UPROPERTY(EditAnywhere, Category = "Puzzle")
	TArray<AActor*> m_AryKeyActor;
	TArray<UPuzzleKey*> m_AryKeyComponent;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FVoidIntBP m_OnPuzzleUnlockCheckedBP;
	FVoidInt m_OnPuzzleUnlockChecked;
	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FVoidIntBP m_OnPuzzleLockCheckedBP;
	FVoidInt m_OnPuzzleLockChecked;
	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FVoidVoidBP m_OnDoorUnlockedBP;
	FVoidVoid m_OnDoorUnlocked;
	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FVoidVoidBP m_OnDoorLockedBP;
	FVoidVoid m_OnDoorLocked;
public:

	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void UnlockDoor();
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void LockDoor();
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void PuzzleChanged();
};
