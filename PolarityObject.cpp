// Fill out your copyright notice in the Description page of Project Settings.


#include "PolarityObject.h"
#include "Engine.h"


const float APolarityObject::STD_POWER = 100.0f;


APolarityObject::APolarityObject()
{

	Root = CreateOptionalDefaultSubobject<USceneComponent>(TEXT("Root"));
	if (Root) {
		RootComponent = Root;
	}

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RedPolar = 0;
	BluePolar = 0;
	PolarityPower = 0;
	Weight = 4.0f;

	bCanPolarize = false;

	ClearAffectedForce();

#if WITH_EDITOR
	InitialPolarity = 0;
#endif // WITH_EDITOR


	bInteractingWithAppointedGroup = false;
	bInteractingWithInfluenceAreas = false;
	bSameInteractionWithInfluenceAreas = true;
	InfluenceRange = 1000.0f;

	CheckPotentialGroupTimeStack = 0;
	CheckPotentialGroupPeriod = FMath::RandRange(0.4f, 0.7f);
}



void APolarityObject::BeginPlay()
{
	if (!PolarMeshIsValid()) {
		Destroy();
	}

	Super::BeginPlay();

	if (!IsNeutral() && bInteractingWithInfluenceAreas)
	{
		bCheckPotentialGroup = true;
		CheckPotentialGroup();
	}
}


void APolarityObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	if (!IsNeutral())
	{
		Interacting(DeltaTime);
		ClearAffectedForce();
	}
}

#if WITH_EDITOR
void APolarityObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const UProperty* PropertyThatChanged = PropertyChangedEvent.MemberProperty;
	if (PropertyThatChanged) {
		if (PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(APolarityObject, InitialPolarity))
		{
			InitPolarity();
		}
		else if (PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(APolarityObject, bCanPolarize))
		{
			if (bCanPolarize)
			{
				SetPolarity(E_Neutral);
			}
			else {
				InitPolarity();
			}
		}
		else if (PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(APolarityObject, Weight))
		{
			CalcPolarityPower();
		}
	}
}

void APolarityObject::InitPolarity()
{
	if (InitialPolarity == 0)
	{
		SetPolarity(E_Neutral);
	}
	else if (InitialPolarity > 0)
	{
		SetPolarity(E_Red, InitialPolarity);
	}
	else
	{
		SetPolarity(E_Blue, InitialPolarity * -1);
	}

	CalcPolarityPower();
}
#endif // WITH_EDITOR


void APolarityObject::PolarizeRed() {
	if (bCanPolarize) {
		RedPolar++;
		OnPolarityValueChanged();
	}
}

void APolarityObject::PolarizeBlue() {
	if (bCanPolarize) {
		BluePolar++;
		OnPolarityValueChanged();
	}
}

bool APolarityObject::DepolarizeRed() {
	if (RedPolar == 0 || !bCanPolarize) {
		return false;
	}

	RedPolar--;
	OnPolarityValueChanged();
	
	return true;
}

bool APolarityObject::DepolarizeBlue() {
	if (BluePolar == 0 || !bCanPolarize) {
		return false;
	}

	BluePolar--;
	OnPolarityValueChanged();
	
	return true;
}

void APolarityObject::CalcPolarityPower()
{
	if (GetPolarity() == E_Neutral)
	{
		PolarityPower = 0;
	}

	int32 PolarGap = FMath::Abs(RedPolar - BluePolar);

	// 극성의 영향력은 5를 넘길 수 없도록 한다.
	if (PolarGap > 5)
	{
		PolarGap = 5;
	}
	
	// 극성차의 영향력은 Log2(X+1) (0 < X < 5) 공식에 따라 0 ~ 2.585의 범위를 가지며 X가 1일 때 1의 값을 얻는다.
	PolarityPower = STD_POWER * FMath::Log2(PolarGap + 1) * Weight;
}

void APolarityObject::OnPolarityValueChanged_Implementation()
{
	UpdateCustomDepthStencilValue();
	
	if (!IsNeutral())
	{
		CalcPolarityPower();

		bCheckPotentialGroup = true;
		if (bInteractingWithInfluenceAreas) {
			CheckPotentialGroup();
		}
	}
	else {
		bCheckPotentialGroup = false;
	}
}

EPolarity APolarityObject::GetPolarity()
{
	if (RedPolar == BluePolar)
	{
		return E_Neutral;
	}
	else if (RedPolar > BluePolar) {
		return E_Red;
	}
	else { // if (RedPolar < BluePolar)
		return E_Blue;
	}
}

bool APolarityObject::IsRed()
{
	if (GetPolarity() == E_Red)
	{
		return true;
	}

	return false;
}

bool APolarityObject::IsBlue()
{
	if (GetPolarity() == E_Blue)
	{
		return true;
	}

	return false;
}

bool APolarityObject::IsNeutral()
{
	if (GetPolarity() == E_Neutral)
	{
		return true;
	}

	return false;
}

bool APolarityObject::IsEmpty()
{
	if (IsNeutral()) {
		if (RedPolar == 0 && BluePolar == 0)
		{
			return true;
		}
	}

	return false;
}


bool APolarityObject::IsOppositePolarity(APolarityObject* Target)
{
	EPolarity MyPolarity = GetPolarity();
	
	if (MyPolarity == E_Blue)
		{
		if (Target->IsRed())
		{
			return true;
		}

		return false;
	}
	else if (MyPolarity == E_Red)
	{
		if (Target->IsBlue())
		{
			return true;
		}

		return false;
	}

	return false;
}


bool APolarityObject::IsSamePolarity(APolarityObject* Target)
{
	if (GetPolarity() == Target->GetPolarity())
	{
		return true;
	}
	
	return false;
}


// 극성이 고정된 오브젝트를 위한 메소드. 기본 극성을 세팅함
void APolarityObject::SetPolarity(EPolarity Polarity, int32 Factor)
{
	if (Polarity == E_Blue)
	{
		RedPolar = 0;
		BluePolar = Factor;
	}
	else if(Polarity == E_Red)
	{
		BluePolar = 0;
		RedPolar = Factor;
	}
	else
	{
		BluePolar = 0;
		RedPolar = 0;
	}

	OnPolarityValueChanged();
}


void APolarityObject::InitPolarMesh(UMeshComponent* NewMesh)
{
	if (NewMesh != NULL)
	{
		PolarityMesh = NewMesh;
	}
}

bool APolarityObject::PolarMeshIsValid() 
{
	return IsValid(PolarityMesh);
}

UMeshComponent* APolarityObject::GetPolarMesh()
{
	return PolarityMesh;
}

void APolarityObject::SetPFModeRenderingOption(bool Value)
{
	if (!PolarMeshIsValid() || (!bCanPolarize && IsNeutral()))
	{
		return;
	}

	PolarityMesh->SetRenderCustomDepth(Value);
	UpdateCustomDepthStencilValue();
}

void APolarityObject::UpdateCustomDepthStencilValue()
{
	if (PolarMeshIsValid())
	{
		int32 value = 0;

		if (IsNeutral())
		{
			value = 1;
		}
		else if (IsRed())
		{
			value = 2;
		}
		else // if IsBlue()
		{
			value = 3;
		}

		PolarityMesh->SetCustomDepthStencilValue(value);
	}
}

void APolarityObject::AddAffectedForce(FVector Force)
{
	AffectedForce += Force;
}

void APolarityObject::ClearAffectedForce()
{
	AffectedForce = FVector::ZeroVector;
}

FVector APolarityObject::ConsumeAffectedForce()
{
	FVector VectorTemp = AffectedForce;
	ClearAffectedForce();
	return VectorTemp;
}

void APolarityObject::Interacting_Implementation(float DeltaTime)
{
	
	if (bInteractingWithAppointedGroup) {// && !bSameInteractionWithInfluenceAreas) {
		AffectAppointedGroup(DeltaTime);
	}

	if (bInteractingWithInfluenceAreas) {
		
		// 상호작용할 가능성이 있는 그룹을 체크한다.
		if (bCheckPotentialGroup)
		{
			CheckPotentialGroupTimeStack += DeltaTime;

			if (CheckPotentialGroupTimeStack > CheckPotentialGroupPeriod)
			{
				CheckPotentialGroup();
				CheckPotentialGroupTimeStack = 0;
				CheckPotentialGroupPeriod = FMath::RandRange(0.3f, 0.6f);
			}
		}
		
		AffectPotentialGroup(DeltaTime);
	}

	if (!AffectedForce.Equals(FVector::ZeroVector)) {
		ApplyAffectedForce(DeltaTime);
	}
}

void APolarityObject::AffectAppointedGroup_Implementation(float DeltaTime)
{
	if (AppointedGroup.Num() <= 0) {
		return;
	}

	for (auto& Target : AppointedGroup)
	{
		AffectByDistance(Target);
	}
}


void APolarityObject::AffectPotentialGroup_Implementation(float DeltaTime) 
{
	if (PotentialGroup.Num() <= 0) {
		return;
	}

	for (auto& Target : PotentialGroup)
	{
		AffectByDistance(Target);
	}
}

void APolarityObject::ApplyAffectedForce_Implementation(float DeltaTime) {}

bool APolarityObject::CanAffect_Implementation(APolarityObject* Target)
{
	if (Target == this || IsNeutral() || Target->IsNeutral())
	{
		return false;
	}
	
	FVector ActorLocation = GetPolarMesh()->GetComponentLocation();
	FVector TargetLocation = Target->GetPolarMesh()->GetComponentLocation();
	float DistanceSquared = FVector::DistSquared(ActorLocation, TargetLocation);

	if (DistanceSquared < FMath::Square(InfluenceRange))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void APolarityObject::CheckPotentialGroup()
{
	PotentialGroup.Empty();

	if (InfluenceAreas.Num() <= 0) {
		return;
	}

	for (auto& Area : InfluenceAreas) {
		
		Area->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		
		
		TArray<struct FOverlapResult> OutOverlaps;
		
		bool isOverlap = GetWorld()->ComponentOverlapMulti(OutOverlaps, Area, Area->GetComponentLocation(), Area->GetComponentRotation(), FComponentQueryParams::DefaultComponentQueryParams, FCollisionObjectQueryParams::DefaultObjectQueryParam);
		
		
		if (isOverlap) {
			for (auto& OutOverlap : OutOverlaps) {
				AActor* OutActor = OutOverlap.GetActor();
				UPrimitiveComponent* OutComp = OutOverlap.GetComponent();
				ECollisionResponse CompCollisionResponse = OutComp->GetCollisionResponseToComponent(PolarityMesh);

				if (CompCollisionResponse == ECR_Overlap || CompCollisionResponse == ECR_Block) {
					APolarityObject* PotentialObject = Cast<APolarityObject>(OutActor);

					if (PotentialObject) {
						if (OutComp == PotentialObject->GetPolarMesh()) {
							PotentialGroup.Emplace(PotentialObject);
						}
					}
				}
			}
		}
		
		
		Area->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 평소에는 NoCollision 상태로 유지
		
	}
}

void APolarityObject::AffectByDistance(APolarityObject* Target) 
{
	if (CanAffect(Target))
	{
		FVector ForceVector = FVector::ZeroVector;

		FVector MyLoc = GetPolarMesh()->GetComponentLocation();
		FVector TargetLoc = Target->GetPolarMesh()->GetComponentLocation();
		if (IsSamePolarity(Target))
		{
			ForceVector = TargetLoc - MyLoc;
		}
		else if (IsOppositePolarity(Target))
		{
			ForceVector = MyLoc - TargetLoc;
		}

		FVector ForceDirection = ForceVector;
		ForceDirection.Normalize();

		float Distance = ForceVector.Size();

		float Power = PolarityPower * ((InfluenceRange - Distance) / InfluenceRange);	// 거리가 가까워질수록(Distance가 작아질수록) 힘이 강해짐

		FVector Force = ForceDirection * Power;
		Target->AddAffectedForce(Force);
	}

}