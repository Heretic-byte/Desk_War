

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PuzzleKey.generated.h"

class UPuzzleDoor;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DESK_WAR_API UPuzzleKey : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPuzzleKey();
protected:
	UPROPERTY(VisibleAnywhere, Category = "Puzzle")
	UPuzzleDoor* m_LinkedDoor;
	UPROPERTY(VisibleAnywhere, Category = "Puzzle")
	int m_bPuzzleUnlocked;


public:
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void InitPuzzleKey(UPuzzleDoor* door);
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void UnlockPuzzle();
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void LockPuzzle();

	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	bool IsKeyUnlocked();
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	UPuzzleDoor* GetLinkedDoor();
};
