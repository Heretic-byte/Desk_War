#include "SpawnObjShow.h"
#include "Components/SkeletalMeshComponent.h"
#include "Actors/ObjectGiver.h"
#include "Datas/USB_Macros.h"
void USpawnObjShow::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	AObjectGiver* Giver = Cast<AObjectGiver>(MeshComp->GetOwner());
	if (!Giver)
	{
		return;
	}
	FVector SpawnPoint = MeshComp->GetSocketLocation("SpawnPoint");
	Giver->ShowActor(SpawnPoint);
	//다른곳에서 힘을 불러줄 방법 필요

}
