#include "LagRecordHelper.h"
#include "Backtrack.h"

void CLagRecordHelper::Set(CTFPlayer* pPlayer, const TickRecord* pRecord)
{
	if (!pPlayer || !pRecord)
		return;

	auto& bones = pPlayer->As<CBaseAnimating>()->m_CachedBoneData();

	m_pPlayer    = pPlayer;
	m_vAbsOrigin = pPlayer->GetAbsOrigin();
	m_vAbsAngles = pPlayer->GetAbsAngles();
	m_vMins      = pPlayer->m_vecMinsPreScaled();
	m_vMaxs      = pPlayer->m_vecMaxsPreScaled();

	// save current cached bone matrices, then hot-swap with the record's bones
	memcpy(m_aBones,     bones.Base(),         sizeof(matrix3x4) * bones.Count());
	memcpy(bones.Base(), pRecord->m_aBones,    sizeof(matrix3x4) * bones.Count());

	pPlayer->SetAbsOrigin(pRecord->m_vOrigin);
	pPlayer->SetAbsAngles(pRecord->m_vAngles);
	pPlayer->m_Collision()->SetCollisionBounds(pRecord->m_vMins, pRecord->m_vMaxs);
}

void CLagRecordHelper::Reset()
{
	if (!m_pPlayer)
		return;

	auto& bones = m_pPlayer->As<CBaseAnimating>()->m_CachedBoneData();

	memcpy(bones.Base(), m_aBones, sizeof(matrix3x4) * bones.Count());

	m_pPlayer->SetAbsOrigin(m_vAbsOrigin);
	m_pPlayer->SetAbsAngles(m_vAbsAngles);
	m_pPlayer->m_Collision()->SetCollisionBounds(m_vMins, m_vMaxs);

	m_pPlayer    = nullptr;
	m_vAbsOrigin = {};
	m_vAbsAngles = {};
	m_vMins      = {};
	m_vMaxs      = {};
}
