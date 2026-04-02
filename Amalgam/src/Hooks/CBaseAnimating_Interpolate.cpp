#include "../SDK/SDK.h"

#include "../Features/Ticks/Ticks.h"

MAKE_SIGNATURE(CBaseAnimating_Interpolate, "client.dll", "48 8B C4 48 89 70 ? F3 0F 11 48", 0x0);

MAKE_HOOK(CBaseAnimating_Interpolate, S::CBaseAnimating_Interpolate(), bool,
	void* rcx, float currentTime)
{
	DEBUG_RETURN(CBaseAnimating_Interpolate, rcx, currentTime);

	// local player: skip during recharge
	if (rcx == H::Entities.GetLocal())
	{
		if (F::Ticks.m_bRecharge)
			return true;
		return CALL_ORIGINAL(rcx, currentTime);
	}

	// explicit interpolation removal
	if (Vars::Visuals::Removals::Interpolation.Value)
		return true;

	return CALL_ORIGINAL(rcx, currentTime);
}
