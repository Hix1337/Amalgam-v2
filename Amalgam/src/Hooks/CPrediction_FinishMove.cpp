#include "../SDK/SDK.h"

MAKE_HOOK(CPrediction_FinishMove, U::Memory.GetVirtual(I::Prediction, 19), void,
	void* rcx, CBasePlayer* player, CUserCmd* cmd, CMoveData* move)
{
	DEBUG_RETURN(CPrediction_FinishMove, rcx, player, cmd, move);

	float pitch = move->m_vecAngles.x;

	if (pitch > 180.0f)
		pitch -= 360.0f;

	pitch = std::clamp(pitch, -90.0f, 90.0f);
	move->m_vecAngles.x = pitch;

	CALL_ORIGINAL(rcx, player, cmd, move);
}
