#include "PuzzleKey.h"
#include "Components/PuzzleDoor.h"
#include "Datas/USB_Macros.h"
// Sets default values for this component's properties



UPuzzleKey::UPuzzleKey()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	m_LinkedDoor = nullptr;
}


// Called when the game starts
void UPuzzleKey::BeginPlay()
{
	Super::BeginPlay();

	m_bPuzzleUnlocked = false;
	if (!m_LinkedDoor)
	{
		PRINTF("%s Doesnt Have Linked Door ! 1", *GetName());
	}
}





void UPuzzleKey::UnlockPuzzle()
{
	if (!m_LinkedDoor)
	{
		PRINTF("%s Doesnt Have Linked Door ! 2", *GetName());
	}

	m_bPuzzleUnlocked = true;

	m_LinkedDoor->PuzzleChanged();
}

void UPuzzleKey::LockPuzzle()
{
	m_bPuzzleUnlocked = false;

	m_LinkedDoor->PuzzleChanged();

}

bool UPuzzleKey::IsKeyUnlocked()
{
	return m_bPuzzleUnlocked;
}

