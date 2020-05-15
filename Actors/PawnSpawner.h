

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"
#include "PawnSpawner.generated.h"



class UDataTable;
UCLASS(hideCategories = (Rendering, Replication, Input, Actor, Base, Collision, Shape, OverlapCapsule,
	Clothing,
	MasterPoseComponent,
	Tags,
	Physics,
	Lighting,
	Sprite,
	Materials,
	Animation,
	Mesh))

class DESK_WAR_API APawnSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	APawnSpawner();
	
private:
#if WITH_EDITOR
	UDataTable* m_DataTable;
	virtual void PostInitProperties() override;
	void SetTablePawnProperty();
	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent) override;
#endif
protected:
	UPROPERTY(VisibleAnywhere, Category = "Spawner")
	UBillboardComponent* m_BilboardForSpawnerLoc;
	UPROPERTY(VisibleAnywhere, Category = "Spawner")
	UBillboardComponent* m_BilboardForSpawnOffset;
	UPROPERTY(VisibleAnywhere,Category="Spawner")
	UStaticMeshComponent* m_MeshBase;
	UPROPERTY(VisibleAnywhere, Category = "Spawner")
	USkeletalMeshComponent* m_MeshSpawnPwnProxy;
	UPROPERTY(VisibleAnywhere, Category = "Spawner")
	USphereComponent* m_PawnAreaProxy;
protected:
	UPROPERTY(EditAnywhere, Category = "Spawner")
	FName m_NameConnectorID;
	UPROPERTY(EditAnywhere, Category = "Spawner")
	bool m_bCanSpawn;
	UPROPERTY(EditAnywhere, Category = "Spawner")
	bool m_bCanSeeSpawner;
	UPROPERTY(EditAnywhere, Category = "Spawner")
	FVector m_SpawnOffset;
	UPROPERTY(EditAnywhere, Category = "Spawner")
	FRotator m_SpawnRotateOffset;
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<APawn> m_ClassPawnToSpawn;
	UPROPERTY()
	APawn* m_SpawnedPawn;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SpawnPawn();// not pool

	
};
