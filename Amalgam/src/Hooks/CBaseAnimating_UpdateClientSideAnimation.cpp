#include "../SDK/SDK.h"

#include "../Features/Backtrack/Backtrack.h"
#include "../Features/Backtrack/LagRecordHelper.h"
#include "../Features/Resolver/Resolver.h"

MAKE_SIGNATURE(CBaseAnimating_UpdateClientSideAnimation, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 8B F8 48 85 C0 74 ? 48 8B 00 48 8B CF FF 90 ? ? ? ? 84 C0 75 ? 33 FF 48 3B DF", 0x0);

MAKE_HOOK(CBaseAnimating_UpdateClientSideAnimation, S::CBaseAnimating_UpdateClientSideAnimation(), void,
	void* rcx)
{
	DEBUG_RETURN(CBaseAnimating_UpdateClientSideAnimation, rcx);

	auto pLocal  = H::Entities.GetLocal();
	auto pPlayer = reinterpret_cast<CTFPlayer*>(rcx);

	// local player: engine handles, except kart/demo where we let it through
	if (pPlayer == pLocal)
	{
		if (!pLocal->InCond(TF_COND_HALLOWEEN_KART) && !I::EngineClient->IsPlayingDemo())
			return;
		CALL_ORIGINAL(rcx);
		return;
	}

	// taunting: engine must drive the taunt animation
	if (pPlayer->InCond(TF_COND_TAUNTING))
	{
		CALL_ORIGINAL(rcx);
		return;
	}

	// inside our own tick-stepping loop — let the original run so anim state advances
	if (G::UpdatingAnims)
	{
		CALL_ORIGINAL(rcx);
		return;
	}

	// --- seo64 rendering approach ---
	// Bones were built at the network packet origin during MakeRecords.
	// GetRenderOrigin() is the engine's smooth interpolated visual position.
	// Shift each bone's translation by (render_origin - record_origin) so the
	// skeleton follows the smooth position without recomputing the whole pose.

	auto pHDR = pPlayer->GetModelPtr();
	if (!pHDR) return;

	auto it = F::Backtrack.m_mRecords.find(pPlayer);
	if (it == F::Backtrack.m_mRecords.end() || it->second.empty())
	{
		// no records yet — fall back to a gated SetupBones so bones aren't garbage
		F::LagRecordHelper.AllowBoneSetup(true);
		pPlayer->InvalidateBoneCache();
		pPlayer->SetupBones(nullptr, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, pPlayer->m_flSimulationTime());
		F::LagRecordHelper.AllowBoneSetup(false);
		return;
	}

	const TickRecord& tRecord = it->second.front();
	if (tRecord.m_bInvalid)
	{
		F::LagRecordHelper.AllowBoneSetup(true);
		pPlayer->InvalidateBoneCache();
		pPlayer->SetupBones(nullptr, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, pPlayer->m_flSimulationTime());
		F::LagRecordHelper.AllowBoneSetup(false);
		return;
	}

	auto& cachedBones = pPlayer->As<CBaseAnimating>()->m_CachedBoneData();
	const Vec3 vRecordOrigin = tRecord.m_vOrigin;
	const Vec3 vRenderOrigin = pPlayer->GetRenderOrigin();

	for (int i = 0; i < cachedBones.Count(); i++)
	{
		const Vec3 vBonePos = { tRecord.m_aBones[i][0][3], tRecord.m_aBones[i][1][3], tRecord.m_aBones[i][2][3] };
		const Vec3 vAdjusted = vRenderOrigin + (vBonePos - vRecordOrigin);
		cachedBones[i][0][3] = vAdjusted.x;
		cachedBones[i][1][3] = vAdjusted.y;
		cachedBones[i][2][3] = vAdjusted.z;
	}

	pPlayer->As<CBaseAnimating>()->SetupBones_AttachmentHelper(pHDR);
}
