#include "../SDK/SDK.h"

MAKE_HOOK(CPrediction_Update2, U::Memory.GetVirtual(I::Prediction, 21), void,
	void* rcx, bool received_new_world_update, bool validframe, int incoming_acknowledged, int outgoing_command)
{
	DEBUG_RETURN(CPrediction_Update2, rcx, received_new_world_update, validframe, incoming_acknowledged, outgoing_command);

	CALL_ORIGINAL(rcx, true, validframe, incoming_acknowledged, outgoing_command);
}
