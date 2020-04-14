// Fill out your copyright notice in the Description page of Project Settings.


#include "PortSkMeshComponent.h"
#include "Datas/USB_Macros.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"


UPortSkMeshComponent::UPortSkMeshComponent(const FObjectInitializer & objInit)
{
	m_MatInitColor = FColor::White;
	m_MatPortFailColor = FColor::Red;
	m_MatPortSuccessColor = FColor::Cyan;

	m_fBlinkDelayFar = 1.0f;
	m_fBlinkDelayNear = 0.5f;
	m_NameMatScalarParam = "Brightness";
	m_fMatBrightness = 0.5f;
	m_fFailImpulsePower = 10000.f;
	m_ConnectableRotation.Yaw = 30.f;
	m_ConnectableRotation.Roll = 5.f;
	m_ConnectableRotation.Pitch = 5.f;
	m_bBlockMoveOnConnected = false;
	m_NameWantMovePoint = "PortPoint";
	m_NameParentBonePortPoint = "PortPoint";
	m_NamePortConnectSocket = "PortPoint";
	m_NamePortConnectStartSocket = "ConnectStart";
	m_NamePortConnectPushPointSocket = "PushPoint";
	m_PortType = EPinPortType::ENoneType;
	m_fEjectPower = 999.f;
	m_fConnectableDistSqr = 25.f;
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundMesh(TEXT("SkeletalMesh'/Game/Meshes/Characters/Port/SK_PortPoint.SK_PortPoint'"));

	if (FoundMesh.Succeeded())
	{
		SetSkeletalMesh(FoundMesh.Object);
	}

	SetCollisionProfileName("Port");
}

void UPortSkMeshComponent::BeginPlay()
{
	Super::BeginPlay();
	m_BlinkMat = GetMaterials()[0];
	m_BlinkMatDynamic = UMaterialInstanceDynamic::Create(m_BlinkMat, this);
	StartBlink(m_fBlinkDelayNear);
}

void UPortSkMeshComponent::InitPort(UPhysicsConstraintComponent * physicsJoint, UPhysicsSkMeshComponent* parentMesh, EPinPortType portType, FName namePinBone)
{
	m_ParentPhysicsConst = physicsJoint;
	m_MeshParentActor = parentMesh;

	if (portType != EPinPortType::ENoneType)
	{
		m_PortType = portType;
	}

	if (namePinBone != NAME_None)
	{
		m_NameParentBonePortPoint = namePinBone;
	}


}

void UPortSkMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (m_bIsBlinkStart)
	{
		m_fCurrentBlinkDelayTimer += DeltaTime;

		if (m_fCurrentBlinkDelayTimer < m_fCurrentBlinkDelay)
		{
			m_fBlinkInterpCache = UKismetMathLibrary::FInterpTo(m_fBlinkInterpCache, m_bIsBlinked ? 0.f : m_fMatBrightness, DeltaTime, 4.0f);
			SetPortMatScalar(m_NameMatScalarParam, m_fBlinkInterpCache);
		}
		else if (m_fCurrentBlinkDelayTimer >= m_fCurrentBlinkDelay)
		{
			m_fCurrentBlinkDelayTimer = 0.f;
			m_bIsBlinked = !m_bIsBlinked;
		}
	}
}

void UPortSkMeshComponent::StartBlink(float blinkDe)
{
	m_bIsBlinked = false;
	m_bIsBlinkStart = true;
	m_fBlinkInterpCache = 0.f;
	m_fCurrentBlinkDelay = blinkDe;
	m_fCurrentBlinkDelayTimer = 0.f;
}

void UPortSkMeshComponent::EndBlink()
{
	m_bIsBlinked = true;
	m_bIsBlinkStart = false;
	m_fBlinkInterpCache = 0.f;
	m_fCurrentBlinkDelay = 0.f;
	m_fCurrentBlinkDelayTimer = 0.f;

	SetPortMatScalar(m_NameMatScalarParam,0.f);
}

void UPortSkMeshComponent::SetPortMatScalar(FName paramName, float scalar)
{
	m_BlinkMatDynamic->SetScalarParameterValue(paramName, scalar);
	SetMaterial(0, m_BlinkMatDynamic);
}

void UPortSkMeshComponent::SetPortMatColor(FName paramName, FLinearColor color)
{
	m_BlinkMatDynamic->SetVectorParameterValue(paramName, color);
	SetMaterial(0, m_BlinkMatDynamic);
}

bool UPortSkMeshComponent::CheckConnectTransform(USceneComponent * connector, bool isConnectorGround)
{
	FRotator PortRot = GetComponentRotation();
	FRotator PinRot = connector->GetComponentRotation();

	float PitchDiff = FMath::Abs(PortRot.Pitch - PinRot.Pitch);
	bool PitchCheck = PitchDiff <= m_ConnectableRotation.Pitch;

	float RollDiff = FMath::Abs(PortRot.Roll - PinRot.Roll);
	bool RollCheck = isConnectorGround ? RollDiff <= m_ConnectableRotation.Roll : true;

	float YawDiff = FMath::Abs(PortRot.Yaw - PinRot.Yaw);
	bool YawCheck = YawDiff <= m_ConnectableRotation.Yaw;

	return PitchCheck && RollCheck&&YawCheck;
}

bool UPortSkMeshComponent::GetBlockMoveOnConnnect()
{
	return m_bBlockMoveOnConnected;
}

FName UPortSkMeshComponent::GetMovePointWant()
{
	return m_NameWantMovePoint;
}

UPhysicsSkMeshComponent * UPortSkMeshComponent::GetParentSkMesh()
{
	return m_MeshParentActor;
}

void UPortSkMeshComponent::OnFocus(UPinSkMeshComponent * aimingPin,bool isConnectorGround)
{
	FRotator PortRot = GetComponentRotation();
	FRotator PinRot = aimingPin->GetComponentRotation();
	
	float RollDiff = FMath::Abs(PortRot.Roll - PinRot.Roll);
	bool RollCheck = isConnectorGround ? RollDiff <= m_ConnectableRotation.Roll : true;

	if (!RollCheck)
	{
		//SetPortMatOriginal();
		//cant
		//red
		SetPortMatColor(m_NameMatVectorParam, m_MatPortFailColor);
	}
	else
	{
		SetPortMatColor(m_NameMatVectorParam, m_MatPortSuccessColor);
	}
}

void UPortSkMeshComponent::OnFocusEnd(UPinSkMeshComponent * aimingPin)
{
	SetPortMatColor(m_NameMatVectorParam, m_MatInitColor);
	SetPortMatScalar(m_NameMatScalarParam, m_fMatBrightness);
}

void UPortSkMeshComponent::Connect(UPinSkMeshComponent * connector)//should call last
{
	DisablePhysicsCollision();
	
	m_ConnectedPin = connector;
	ConstraintPinPort();
	m_OnConnected.Broadcast(m_ConnectedPin);
	EnablePhysicsCollision();
	SetPortMatColor(m_NameMatVectorParam, m_MatInitColor);
	SetPortMatScalar(m_NameMatScalarParam, m_fMatBrightness);
	EndBlink();
}

void UPortSkMeshComponent::ConstraintPinPort()
{
	m_ParentPhysicsConst->SetConstrainedComponents(m_MeshParentActor, NAME_None, m_ConnectedPin, NAME_None);
}

bool UPortSkMeshComponent::Disconnect()
{
	if (!m_ConnectedPin)
	{
		return false;
	}
	m_OnDisconnected.Broadcast(m_ConnectedPin);
	m_ParentPhysicsConst->BreakConstraint();
	m_ConnectedPin = nullptr;

	m_MeshParentActor->AddImpulse(GetForwardVector()*m_fEjectPower);

	//blink의 시작과끝은 범위 들어오는것으로 처리해줘야함
	return true;
}

bool UPortSkMeshComponent::IsConnected()
{
	return m_ConnectedPin;
}


void UPortSkMeshComponent::EnablePhysicsCollision()
{
	m_MeshParentActor->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Block);
}

void UPortSkMeshComponent::DisablePhysicsCollision()
{
	m_MeshParentActor->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);
}

EPinPortType UPortSkMeshComponent::GetPortType() const
{
	return _inline_GetPortType();
}

void UPortSkMeshComponent::FailConnection(const FHitResult & hitResult)
{
	m_MeshParentActor->AddImpulseAtLocation((GetUpVector() + GetForwardVector())*m_fFailImpulsePower, hitResult.ImpactPoint);
}


