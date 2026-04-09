#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBaseEntity_Teleported, "client.dll", "48 89 5C 24 08 44 8B 81", 0x0);
MAKE_SIGNATURE(CBaseAnimating_SetupBones_Teleported_Call, "client.dll", "84 C0 75 18 41 0F B6 86 AC 00 00 00", 0x0);

MAKE_HOOK(CBaseEntity_Teleported, S::CBaseEntity_Teleported(), bool,
	void* rcx)
{
	DEBUG_RETURN(CBaseEntity_Teleported, rcx);

	// if we have ik active we should force its targets to clear always
	if (uintptr_t(_ReturnAddress()) == S::CBaseAnimating_SetupBones_Teleported_Call())
		return true;

	return CALL_ORIGINAL(rcx);
}
