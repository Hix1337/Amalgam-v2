#include "../SDK/SDK.h"
#include "../Features/Prediction/CompressionHandler.h"

MAKE_SIGNATURE(CBaseEntity_PreEntityPacketReceived, "client.dll", "40 53 48 83 EC 20 48 8B 01 48 8B D9 85 D2", 0x0);

MAKE_HOOK(CBaseEntity_PreEntityPacketReceived, S::CBaseEntity_PreEntityPacketReceived(), void,
	void* rcx, int commands_acknowledged)
{
	DEBUG_RETURN(CBaseEntity_PreEntityPacketReceived, rcx, commands_acknowledged);

	auto* pEntity = reinterpret_cast<CBaseEntity*>(rcx);

	if (pEntity->entindex() != I::EngineClient->GetLocalPlayer())
		return CALL_ORIGINAL(rcx, commands_acknowledged);

	auto* pPlayer = reinterpret_cast<CBasePlayer*>(rcx);
	F::CompressionHandler.OnPreEntityPacketReceived(pPlayer, commands_acknowledged);

	CALL_ORIGINAL(rcx, commands_acknowledged);
}
