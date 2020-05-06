

#pragma once

#include "CoreMinimal.h"
#include "Actors/SingleDoor.h"
#include "TwinDoor.generated.h"

UCLASS()
class DESK_WAR_API ATwinDoor : public ASingleDoor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATwinDoor();
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door")
	TArray<UMaterialInstanceDynamic*> m_AryMatBrightnessSecond;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door")
	TArray<UMaterialInstanceDynamic*> m_AryMatGaugeSecond;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Door")
	USkeletalMeshComponent* m_MeshSecondDoor;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door")
	FRotator m_DoorOpenRotSecond;
	UPROPERTY()
	FRotator m_InitDoorRotSecond;

protected:
	virtual void BeginPlay() override;
	virtual void OpenDoor() override;
	virtual void CloseDoor() override;
};
