#include "ConnectablePawn.h"
#include "Kismet/GameplayStatics.h"
#include "Managers/USB_GameMode.h"
#include "Datas/ConnectablePawnData.h"
#include "UObjects/ConnectionBehavior.h"
// Sets default values
AConnectablePawn::AConnectablePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_PawnID = NAME_None;
	
}

// Called when the game starts or when spawned
void AConnectablePawn::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(false);

	if (m_PawnID != NAME_None)
	{
		SetConnectPawn(m_PawnID);
	}
}

void AConnectablePawn::OnConnected(IConnectable * portTarget)
{
}

void AConnectablePawn::OnDisconnected(IConnectable * pinTarget)
{
}

void AConnectablePawn::SetConnectPawn(FName pawnID)
{
	AUSB_GameMode* GameMode=Cast<AUSB_GameMode>( UGameplayStatics::GetGameMode(GetWorld()));

	const auto& PawnData= GameMode->GetConnectPawnData(pawnID);

	m_PinType = PawnData.m_PinType;
	m_PortType = PawnData.m_PortType;
	m_PawnID = PawnData.m_NameID;
	m_PawnName = PawnData.m_ShowingName;

	m_MeshMainBody->SetSkeletalMesh(PawnData.m_MeshPawnMainBody);
	m_MeshPort->SetSkeletalMesh(PawnData.m_MeshPortBody);

	m_MeshPort->SetRelativeLocation(PawnData.m_PortRelativeLoc);
	m_MeshPort->SetRelativeRotation(PawnData.m_PortRelativeRot);

	m_Sphere->SetSphereRadius(PawnData.m_fInteractRadius);

	m_ConnectionBehav = NewObject<UConnectionBehavior>( PawnData.m_ConnectBehav->GetDefaultObject(), PawnData.m_ConnectBehav);
	//���͸��� ���� ����,�׹ۿ� �ִ°͵�
	m_StartLocation = GetActorLocation();

	if (!PawnData.m_bIsAI)
	{
		return;
	}

	SetActorTickEnabled(true);
	//pawn sensing

	//movement

	//��� ������ ���?

	//�����غ��� ���͸������ ���� ����;ƴѰ�

	//�ϴ��� ������

	//���� ����� �ý��ۤ�;

	//Ŀ�ؼ� �������̽� �� ���� ���� Ŀ���� ����

	//Ŀ�ؼ� ��Ŀ�ؼ� ��� ���� ��ü�� �÷��̾��.
}

