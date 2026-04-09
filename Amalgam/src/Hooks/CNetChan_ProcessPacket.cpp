#include "../SDK/SDK.h"

MAKE_SIGNATURE(CNetChan_ProcessPacket, "engine.dll", "44 88 44 24 18 48 89 54 24 10 53", 0x0);

MAKE_HOOK(CNetChan_ProcessPacket, S::CNetChan_ProcessPacket(), void,
	void* rcx, void* packet, bool header)
{
	DEBUG_RETURN(CNetChan_ProcessPacket, rcx, packet, header);

	CALL_ORIGINAL(rcx, packet, header);

	I::EngineClient->FireEvents();
}
