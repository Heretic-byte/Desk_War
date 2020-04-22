

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
public:
protected:
	UPROPERTY(VisibleAnywhere, Category = "Puzzle")
	UPuzzleDoor* m_LinkedDoor;
	UPROPERTY(VisibleAnywhere, Category = "Puzzle")
	int m_bPuzzleUnlocked;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void UnlockPuzzle();
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void LockPuzzle();

	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	bool IsKeyUnlocked();
		
};
