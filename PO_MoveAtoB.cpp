// Fill out your copyright notice in the Description page of Project Settings.


#include "PO_MoveAtoB.h"
#include "Engine.h"
#include "UObject/ConstructorHelpers.h"


// Sets default values
APO_MoveAtoB::APO_MoveAtoB()
{
	Home = CreateOptionalDefaultSubobject<USceneComponent>(TEXT("Home"));
	if (Home)
	{
		Home->SetupAttachment(RootComponent);
	}

	Destination = CreateOptionalDefaultSubobject<USceneComponent>(TEXT("Destination"));
	if (Destination)
	{
		Destination->SetupAttachment(RootComponent);
	}

#if WITH_EDITORONLY_DATA
	// Home과 Destination에 Sprite를 달아준다.
	if (Home || Destination)
	{
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> TargetIconObject;
			FConstructorStatics()
				: TargetIconObject(TEXT("/Engine/EditorMaterials/TargetIcon"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;
		if (Home)
		{
			HomeSpriteComponent = CreateOptionalDefaultSubobject<UBillboardComponent>(TEXT("HomeSprite"));
			if (HomeSpriteComponent) {
				HomeSpriteComponent->Sprite = ConstructorStatics.TargetIconObject.Get();
				HomeSpriteComponent->RelativeScale3D = FVector(0.35f, 0.35f, 0.35f);
				HomeSpriteComponent->bIsScreenSizeScaled = true;
				HomeSpriteComponent->SetupAttachment(Home);
			}
		}

		if (Destination)
		{
			DestinationSpriteComponent = CreateOptionalDefaultSubobject<UBillboardComponent>(TEXT("DestinationSprite"));
			if (DestinationSpriteComponent) {
				DestinationSpriteComponent->Sprite = ConstructorStatics.TargetIconObject.Get();
				DestinationSpriteComponent->RelativeScale3D = FVector(0.35f, 0.35f, 0.35f);
				DestinationSpriteComponent->bIsScreenSizeScaled = true;
				DestinationSpriteComponent->SetupAttachment(Destination);
			}
		}
		
	}
#endif // WITH_EDITORONLY_DATA


	Direction = FVector::ZeroVector;
	Alpha = 0;
	bToDestination = true;
}

// Called when the game starts or when spawned
void APO_MoveAtoB::BeginPlay()
{
	Super::BeginPlay();

	CalcDirection();
	CalcMaxAlpha();
}


void APO_MoveAtoB::CalcDirection()
{
	FVector HomeLoc = Home->GetComponentLocation();
	FVector DestinationLoc = Destination->GetComponentLocation();

	FVector DesiredDirection = DestinationLoc - HomeLoc;
	DesiredDirection.Normalize();

	Direction = DesiredDirection;
}

void APO_MoveAtoB::CalcMaxAlpha()
{
	FVector HomeLoc = Home->GetComponentLocation();
	FVector DestinationLoc = Destination->GetComponentLocation();

	MaxAlpha = FVector::Dist(HomeLoc, DestinationLoc);
}


void APO_MoveAtoB::ApplyAffectedForce_Implementation(float DeltaTime)
{
	if (MaxAlpha < 1) {
		return;
	}

	FVector Force = ConsumeAffectedForce();
	FVector NetForce = Force.ProjectOnTo(Direction);	// Direction 방향으로 수선을 내려 Direction 외의 힘을 소실시킴

	bToDestination = IsForceToDestination(NetForce);
	MoveMesh(NetForce, DeltaTime);
}


void APO_MoveAtoB::MoveMesh(FVector NetForce, float DeltaTime, int32 Iterations)
{
	if (DeltaTime == 0 || Iterations > 5 || NetForce.Equals(FVector::ZeroVector)) {
		return;
	}

	FVector OldLoc = GetPolarMesh()->GetComponentLocation();

	FVector DeltaVector = NetForce * DeltaTime;

	float NewAlpha;
	
	if (bToDestination) {
		NewAlpha = Alpha + DeltaVector.Size();
	}
	else
	{	// if false => ToHome
		NewAlpha = Alpha - DeltaVector.Size();
	}

	bool bForcedAlphaAdjustment = false;	// NewAlpha가 범위를 초과해서 강제로 바뀌었는지 판별
	FVector DesiredLoc = PolarityMesh->GetComponentLocation() + DeltaVector;

	if (NewAlpha < 0) {
		NewAlpha = 0;
		bForcedAlphaAdjustment = true;
		DesiredLoc = Home->GetComponentLocation();
	}
	else if (MaxAlpha < NewAlpha) {
		NewAlpha = MaxAlpha;
		bForcedAlphaAdjustment = true;
		DesiredLoc = Destination->GetComponentLocation();
	}

	PolarityMesh->SetWorldLocation(DesiredLoc);

	if (CheckCollision()) {
		PolarityMesh->SetWorldLocation(OldLoc);

		if (!bForcedAlphaAdjustment) {
			MoveMesh(NetForce, DeltaTime / 2, ++Iterations);
		}

		return;
	}
	
	Alpha = NewAlpha;
}

bool APO_MoveAtoB::IsForceToDestination(FVector Force)
{
	// Direction 과 Force의 내적이 0보다 크다면 방향이 같은 것이므로 Home To Destination
	return FVector::DotProduct(Force, Direction) > 0.0f;;
}

// 경우에 따라 다른 충돌로직이 필요하다면 클래스를 상속받아 새로운 충돌로직을 정의하여야 한다.
bool APO_MoveAtoB::CheckCollision_Implementation()
{
	// 오버라이드시 반드시 포함!!
	if (!bCollisionCheck) {
		return false;
	}

	TArray<struct FOverlapResult> OutOverlaps;

	bool isHit = GetWorld()->ComponentOverlapMulti(OutOverlaps, PolarityMesh, PolarityMesh->GetComponentLocation(), PolarityMesh->GetComponentRotation(), FComponentQueryParams::DefaultComponentQueryParams, FCollisionObjectQueryParams::DefaultObjectQueryParam);
	
	for (auto& OutOverlap : OutOverlaps) {
		UPrimitiveComponent* OutComp = OutOverlap.GetComponent();
		ECollisionResponse CollisionResponse = OutComp->GetCollisionResponseToComponent(PolarityMesh);

		if (CollisionResponse == ECR_Block) {
			// Overlap해도 움직임이 멈추지 않도록 하고 싶은 액터에 경우 DontStopMove Tag를 심어두면 된다.
			if (OutOverlap.GetActor()->ActorHasTag("DontStopMove")) {
				continue;
			}

			return true;
		}
	}

	return false;
}