#include "../SDK/SDK.h"

// seo64's signature is a call-site relref (E8 ??). Resolve the relative displacement
// to get the actual function address — Amalgam's MAKE_SIGNATURE only does byte offsets,
// so we resolve the call manually here.
static uintptr_t GetCheckForSequenceChangeAddr()
{
	static uintptr_t cached = 0;
	if (!cached)
	{
		const uintptr_t callSite = U::Memory.FindSignature("client.dll", "E8 ? ? ? ? 41 8B 86 ? ? ? ? 49 8B CF");
		if (callSite)
			cached = *reinterpret_cast<int32_t*>(callSite + 1) + callSite + 5;
	}
	return cached;
}

MAKE_HOOK(CSequenceTransitioner_CheckForSequenceChange, GetCheckForSequenceChangeAddr(), void,
	void* rcx, CStudioHdr* hdr, int nCurSequence, bool bForceNewSequence, bool bInterpolate)
{
	DEBUG_RETURN(CSequenceTransitioner_CheckForSequenceChange, rcx, hdr, nCurSequence, bForceNewSequence, bInterpolate);

	// seo64: force bInterpolate=false so sequence transitions snap immediately
	// instead of smearing through an intermediate blended pose.
	CALL_ORIGINAL(rcx, hdr, nCurSequence, bForceNewSequence, false);
}
