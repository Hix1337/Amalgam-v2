#include "../SDK/SDK.h"

MAKE_SIGNATURE(CClientState_GetClientInterpAmount, "engine.dll", "48 83 EC ? 48 8B 0D ? ? ? ? 48 85 C9 75", 0x0);

MAKE_HOOK(CClientState_GetClientInterpAmount, S::CClientState_GetClientInterpAmount(), float,
	CClientState* rcx)
{
	DEBUG_RETURN(CClientState_GetClientInterpAmount, rcx);

	G::Lerp = CALL_ORIGINAL(rcx);

	// Mirror the client.dll hook: only zero out the engine timeline when lerp or
	// interpolation removal is explicitly requested. Otherwise the engine's interp
	// history has no buffered states to blend between, so networked animation layers
	// (m_flCycle, m_flWeight) snap every tick instead of interpolating smoothly.
	if (Vars::Visuals::Removals::Lerp.Value || Vars::Visuals::Removals::Interpolation.Value)
		return 0.f;

	return G::Lerp;
}