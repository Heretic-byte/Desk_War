// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DialogueComponent.h"
#include "TestSceneComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DESK_WAR_API UTestSceneComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTestSceneComponent(const FObjectInitializer& obj);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		UPROPERTY(VisibleAnywhere)
		UDialogueComponent* m_DialCompo;
		UPROPERTY(VisibleAnywhere)
		UCapsuleComponent* m_CapsuleCompo;
};
