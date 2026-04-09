#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBasePlayer_PostThink, "client.dll", "48 89 5C 24 10 57 48 83 EC 20 48 8B 1D ? ? ? ? 48 8B F9", 0x0);

MAKE_HOOK(CBasePlayer_PostThink, S::CBasePlayer_PostThink(), void,
	void* rcx)
{
	DEBUG_RETURN(CBasePlayer_PostThink, rcx);

	if (F::EnginePrediction.m_bInPrediction)
		return;

	CALL_ORIGINAL(rcx);
}
