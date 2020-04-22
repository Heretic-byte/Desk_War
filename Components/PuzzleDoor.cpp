


#include "PuzzleDoor.h"
#include "Components/PuzzleKey.h"

// Sets default values for this component's properties
UPuzzleDoor::UPuzzleDoor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	m_nDoorUnlockCount = -1;
}

// Called when the game starts or when spawned
void UPuzzleDoor::BeginPlay()
{
	Super::BeginPlay();

	if (m_AryKeyActor.Num() < 1)
	{
		PRINTF("%s - Set Key Actor for Puzzle Door !!!!", *GetName());
	}
	for (auto PuzzleKey : m_AryKeyActor)
	{
		if (!PuzzleKey)
		{
			PRINTF("%s - Some Key Ary NULL !!!!", *GetName());
			return;
		}
	}

}

// Called every frame

void UPuzzleDoor::PuzzleChanged()// call evty key changed
{
	int Count = 0;

	for (int i = 0; i < m_AryKeyActor.Num(); i++)
	{
		if (m_AryKeyActor[i]->IsKeyUnlocked())
		{
			m_OnPuzzleUnlockCheckedBP.Broadcast(i);
			m_OnPuzzleUnlockChecked.Broadcast(i);
			Count++;
		}
	}

	if ((m_nDoorUnlockCount > 0 && m_nDoorUnlockCount < m_AryKeyActor.Num()) || m_AryKeyActor.Num() == Count)
	{
		UnlockDoor();
	}
}

void UPuzzleDoor::UnlockDoor()
{
	m_OnDoorUnlockedBP.Broadcast();
	m_OnDoorUnlocked.Broadcast();
}

void UPuzzleDoor::LockDoor()
{
	m_OnDoorLockedBP.Broadcast();
	m_OnDoorLocked.Broadcast();
}
