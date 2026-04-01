#pragma once
#include "../../SDK/SDK.h"

struct TickRecord;

// Port of seo64's LagRecordMatrixHelper.
//
// Two jobs:
//   1. Gate — IsBoneSetupAllowed() / AllowBoneSetup() replaces the scattered
//      m_bSettingUpBones flag so the SetupBones hook has one clean control point.
//
//   2. State swap — Set() saves the player's current world state and applies a
//      TickRecord's state (bones into CachedBoneData, abs origin/angles, collision
//      bounds). Reset() restores everything. Lets aimbot / rendering code temporarily
//      put a player at a historical position without touching the real entity state.

class CLagRecordHelper
{
private:
	CTFPlayer*  m_pPlayer    = nullptr;
	Vec3        m_vAbsOrigin = {};
	Vec3        m_vAbsAngles = {};
	Vec3        m_vMins      = {};
	Vec3        m_vMaxs      = {};
	matrix3x4   m_aBones[128] = {};

	bool m_bBoneSetupAllowed = false;

public:
	// Apply a TickRecord's world state onto pPlayer.
	// Always pair with Reset() — never nest.
	void Set(CTFPlayer* pPlayer, const TickRecord* pRecord);

	// Restore the state saved by the last Set() call.
	void Reset();

	void AllowBoneSetup(bool bAllow) { m_bBoneSetupAllowed = bAllow; }
	bool IsBoneSetupAllowed() const  { return m_bBoneSetupAllowed; }
};

ADD_FEATURE(CLagRecordHelper, LagRecordHelper);
