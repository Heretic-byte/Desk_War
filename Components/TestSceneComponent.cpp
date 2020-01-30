// Fill out your copyright notice in the Description page of Project Settings.


#include "TestSceneComponent.h"

// Sets default values for this component's properties
UTestSceneComponent::UTestSceneComponent(const FObjectInitializer& obj):Super(obj)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;



	m_CapsuleCompo = CreateDefaultSubobject<UCapsuleComponent>(TEXT("ASD"));

	//m_CapsuleCompo->SetupAttachment(GetOwner()->GetRootComponent());

	m_DialCompo = CreateDefaultSubobject<UDialogueComponent>(TEXT("AS32D"));
}


// Called when the game starts
void UTestSceneComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTestSceneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

