#include "../SDK/SDK.h"

MAKE_SIGNATURE(CMultiPlayerAnimState_ComputePoseParam_AimYaw, "client.dll", "40 53 55 56 57 48 83 EC 78 48 8B 01", 0x0);

MAKE_HOOK(CMultiPlayerAnimState_ComputePoseParam_AimYaw, S::CMultiPlayerAnimState_ComputePoseParam_AimYaw(), void,
	void* rcx, CStudioHdr* pStudioHdr)
{
	DEBUG_RETURN(CMultiPlayerAnimState_ComputePoseParam_AimYaw, rcx, pStudioHdr);

	CALL_ORIGINAL(rcx, pStudioHdr);

	auto* pAnimState = reinterpret_cast<CMultiPlayerAnimState*>(rcx);

	if (!pAnimState)
		return;

	CBasePlayer* pPlayer = pAnimState->m_pEntity;

	if (!pPlayer)
		return;

	QAngle vAngles = pPlayer->GetAbsAngles();
	vAngles.y = pAnimState->m_flCurrentFeetYaw;
	pPlayer->SetAbsAngles(vAngles);
}
