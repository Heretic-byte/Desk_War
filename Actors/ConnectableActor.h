

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PhysicsSkMeshComponent.h"
#include "Components/PortSkMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PinSkMeshComponent.h"
#include "Components/AudioComponent.h"
#include "ConnectableActor.generated.h"

UCLASS()
class DESK_WAR_API AConnectableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AConnectableActor();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	UPhysicsConstraintComponent* m_PhysicsCons;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UPhysicsSkMeshComponent* m_Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UPortSkMeshComponent* m_MeshPort;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USphereComponent* m_Sphere;//for check player dist
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAudioComponent* m_Audio;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	virtual void OnConnected(UPinSkMeshComponent* skComp);
	virtual void OnDisconnected(UPinSkMeshComponent* skComp);
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
