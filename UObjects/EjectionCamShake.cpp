


#include "EjectionCamShake.h"

UEjectionCamShake::UEjectionCamShake(const FObjectInitializer& objInit):Super(objInit)
{
	OscillationDuration = 0.25f;
	OscillationBlendInTime = 0.05f;
	OscillationBlendOutTime = 0.05f;

	//RotOscillation.Pitch.Amplitude = 1.f;
	//RotOscillation.Pitch.Frequency = 1.f;
	LocOscillation.X.Amplitude =  85.f;
	LocOscillation.X.Frequency =  10.f;
}