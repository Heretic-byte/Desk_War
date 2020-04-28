#include "CamTimer.h"


UCamTimer::UCamTimer()
{
	m_fShowTimer = 0.f;
	m_PrevCameraActor = nullptr;
	m_fBlendTime = 0.3f;
	PrimaryComponentTick.bCanEverTick = true;
}

void UCamTimer::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(false);
}

void UCamTimer::ShowCamera(APlayerController* con, float duration)
{
	m_PrevCamController = con;
	m_PrevCameraActor= m_PrevCamController->GetViewTarget();

	m_PrevCamController->SetViewTargetWithBlend(GetOwner(), m_fBlendTime);

	m_fShowTimer = duration;

	SetComponentTickEnabled(true);
}

void UCamTimer::HideCamera()
{
	m_PrevCamController->SetViewTargetWithBlend(m_PrevCameraActor, m_fBlendTime);

	m_fShowTimer = 0.f;

	SetComponentTickEnabled(false);
}

void UCamTimer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!m_PrevCamController)
	{
		return;
	}

	if (m_fShowTimer > 0.f)
	{
		m_fShowTimer -= DeltaTime;

	}
	else
	{
		HideCamera();
	}
}
