#include "../SDK/SDK.h"

#include "../Features/Ticks/Ticks.h"
#include "../Features/Resolver/Resolver.h"

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

	// enemy: explicit interpolation removal → skip entirely
	if (Vars::Visuals::Removals::Interpolation.Value)
		return true;

	// resolver target: instead of skipping interpolation (janky) or letting it run freely
	// (flicker from blending stale pre-correction angles), force EF_NOINTERP for this call.
	// the engine snaps to current corrected state, clears the stale sample history, and next
	// frame blends smoothly from the corrected position — no flicker, no stop-motion.
	auto pAnimating = reinterpret_cast<CBaseAnimating*>(rcx);
	if (F::Resolver.GetAngles(reinterpret_cast<CTFPlayer*>(rcx)))
	{
		const int nSavedEffects = pAnimating->m_fEffects();
		pAnimating->m_fEffects() |= 8; // EF_NOINTERP
		const bool bResult = CALL_ORIGINAL(rcx, currentTime);
		pAnimating->m_fEffects() = nSavedEffects;
		return bResult;
	}

	return CALL_ORIGINAL(rcx, currentTime);
}