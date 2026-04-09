#include "../SDK/SDK.h"
#include "../Features/Prediction/CompressionHandler.h"

MAKE_SIGNATURE(CBaseEntity_PostNetworkDataReceived, "client.dll",
	"48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 54 41 56 48 81 EC 10 01 00 00", 0x0);

MAKE_HOOK(CBaseEntity_PostNetworkDataReceived, S::CBaseEntity_PostNetworkDataReceived(), bool,
	void* rcx, int commands_acknowledged)
{
	DEBUG_RETURN(CBaseEntity_PostNetworkDataReceived, rcx, commands_acknowledged);

	auto* pEntity = reinterpret_cast<CBaseEntity*>(rcx);

	if (commands_acknowledged <= 0 || pEntity->entindex() != I::EngineClient->GetLocalPlayer())
		return CALL_ORIGINAL(rcx, commands_acknowledged);

	auto* pPlayer = reinterpret_cast<CBasePlayer*>(rcx);
	if (!pPlayer->IsAlive())
		return CALL_ORIGINAL(rcx, commands_acknowledged);

	F::CompressionHandler.OnPostNetworkDataReceived(pPlayer, commands_acknowledged - 1);

	return CALL_ORIGINAL(rcx, commands_acknowledged);
}
