#include "CompressionHandler.h"

// seo64's RestoreData sig is a call-site relref — resolve the relative call
// same pattern as CSequenceTransitioner_CheckForSequenceChange
static uintptr_t GetRestoreDataAddr()
{
	static uintptr_t cached = 0;
	if (!cached)
	{
		const uintptr_t callSite = U::Memory.FindSignature("client.dll", "E8 ? ? ? ? 48 8B CF 8B D8 E8 ? ? ? ? 48 85 C0");
		if (callSite)
			cached = *reinterpret_cast<int32_t*>(callSite + 1) + callSite + 5;
	}
	return cached;
}

void CCompressionHandler::Init()
{
	// resolve RestoreData
	const uintptr_t addr = GetRestoreDataAddr();
	if (addr)
		m_fnRestoreData = reinterpret_cast<RestoreDataFn>(addr);

	// resolve offsets at runtime via Amalgam's netvar system instead of hardcoding seo64's offsets
	const int off_FallVelocity = U::NetVars.GetNetVar("CBasePlayer", "m_flFallVelocity");
	const int off_PunchAngle = U::NetVars.GetNetVar("CBasePlayer", "m_vecPunchAngle");
	const int off_PunchAngleVel = U::NetVars.GetNetVar("CBasePlayer", "m_vecPunchAngleVel");
	const int off_StepSize = U::NetVars.GetNetVar("CBasePlayer", "m_flStepSize");
	const int off_ViewOffset = U::NetVars.GetNetVar("CBasePlayer", "m_vecViewOffset[0]");
	const int off_DuckTime = U::NetVars.GetNetVar("CBasePlayer", "m_flDucktime");
	const int off_DuckJumpTime = U::NetVars.GetNetVar("CBasePlayer", "m_flDuckJumpTime");
	const int off_JumpTime = U::NetVars.GetNetVar("CBasePlayer", "m_flJumpTime");

	m_vCompressedVars.emplace_back(std::make_unique<FloatNetvar>(off_FallVelocity, 1.0f / AssignRangeMultiplier(17, 4096.0 - (-4096.0))));
	m_vCompressedVars.emplace_back(std::make_unique<VectorNetvar>(off_PunchAngle, DIST_EPSILON));
	m_vCompressedVars.emplace_back(std::make_unique<VectorNetvar>(off_PunchAngleVel, DIST_EPSILON));
	m_vCompressedVars.emplace_back(std::make_unique<FloatNetvar>(off_StepSize, 1.0f / AssignRangeMultiplier(16, 128.0)));

	// view offset — each component has different bit count and range
	m_vCompressedVars.emplace_back(std::make_unique<FloatNetvar>(off_ViewOffset + 0, 1.0f / AssignRangeMultiplier(8, 32.0 - (-32.0))));
	m_vCompressedVars.emplace_back(std::make_unique<FloatNetvar>(off_ViewOffset + 4, 1.0f / AssignRangeMultiplier(8, 32.0 - (-32.0))));
	m_vCompressedVars.emplace_back(std::make_unique<FloatNetvar>(off_ViewOffset + 8, 1.0f / AssignRangeMultiplier(20, 256.0)));

	m_vCompressedVars.emplace_back(std::make_unique<FloatNetvar>(off_DuckTime, 1.0f / AssignRangeMultiplier(12, 2048.0)));
	m_vCompressedVars.emplace_back(std::make_unique<FloatNetvar>(off_DuckJumpTime, 1.0f / AssignRangeMultiplier(12, 2048.0)));
	m_vCompressedVars.emplace_back(std::make_unique<FloatNetvar>(off_JumpTime, 1.0f / AssignRangeMultiplier(12, 2048.0)));
}

void CCompressionHandler::OnPreEntityPacketReceived(CBasePlayer* pPlayer, int iCommandSlot)
{
	if (iCommandSlot > 0 && pPlayer && pPlayer->IsAlive())
	{
		m_bReceivedPacket = true;

		// restore entity to the predicted state for this command slot
		if (m_fnRestoreData)
			m_fnRestoreData(pPlayer, "", iCommandSlot - 1, PC_EVERYTHING);

		for (auto& pVar : m_vCompressedVars)
			pVar->OnPreEntityPacketReceived(pPlayer);
	}
}

void CCompressionHandler::OnPostNetworkDataReceived(CBasePlayer* pPlayer, int iCommandsAcknowledged)
{
	if (!m_bReceivedPacket)
		return;

	for (auto& pVar : m_vCompressedVars)
		pVar->OnPostNetworkDataReceived(pPlayer);
}

void CCompressionHandler::OnLevelInit()
{
	m_bReceivedPacket = false;

	for (auto& pVar : m_vCompressedVars)
		pVar->Reset();
}

#pragma warning(push)
#pragma warning(disable : 4305)

float CCompressionHandler::AssignRangeMultiplier(int nBits, double range)
{
	unsigned long iHighValue;
	if (nBits == 32)
		iHighValue = 0xFFFFFFFE;
	else
		iHighValue = ((1 << static_cast<unsigned long>(nBits)) - 1);

	float fHighLowMul = static_cast<float>(iHighValue / range);
	if (CloseEnough(static_cast<float>(range), 0.f))
		fHighLowMul = static_cast<float>(iHighValue);

	if (static_cast<unsigned long>(fHighLowMul * range) > iHighValue ||
		(fHighLowMul * range) > static_cast<double>(iHighValue))
	{
		float multipliers[] = { 0.9999f, 0.99f, 0.9f, 0.8f, 0.7f };
		int i;
		for (i = 0; i < 4; i++)
		{
			fHighLowMul = static_cast<float>(iHighValue / range) * multipliers[i];
			if (static_cast<unsigned long>(fHighLowMul * range) <= iHighValue &&
				(fHighLowMul * range) <= static_cast<double>(iHighValue))
				break;
		}

		if (i == 4)
			return 1.f;
	}

	return fHighLowMul;
}

#pragma warning(pop)

bool CCompressionHandler::CloseEnough(float a, float b, float epsilon)
{
	return fabsf(a - b) <= epsilon;
}

// FloatNetvar
FloatNetvar::FloatNetvar(int iOffset, float flTolerance)
{
	m_iOffset = iOffset;
	m_flTolerance = flTolerance;
}

void FloatNetvar::OnPreEntityPacketReceived(CBasePlayer* pPlayer)
{
	m_flOldValue = *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(pPlayer) + m_iOffset);
}

void FloatNetvar::OnPostNetworkDataReceived(CBasePlayer* pPlayer)
{
	float flNewValue = *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(pPlayer) + m_iOffset);
	float flOldValue = m_flOldValue;

	if (IsInTolerance(&flOldValue, &flNewValue))
		*reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(pPlayer) + m_iOffset) = flOldValue;
}

bool FloatNetvar::IsInTolerance(void* pOldValue, void* pNewValue)
{
	float flOld = *reinterpret_cast<float*>(pOldValue);
	float flNew = *reinterpret_cast<float*>(pNewValue);
	return fabsf(flNew - flOld) <= m_flTolerance;
}

void FloatNetvar::Reset()
{
	m_flOldValue = 0.f;
}

// VectorNetvar
VectorNetvar::VectorNetvar(int iOffset, float flTolerance)
{
	m_iOffset = iOffset;
	m_flTolerance = flTolerance;
}

void VectorNetvar::OnPreEntityPacketReceived(CBasePlayer* pPlayer)
{
	m_vOldValue = *reinterpret_cast<Vec3*>(reinterpret_cast<uintptr_t>(pPlayer) + m_iOffset);
}

void VectorNetvar::OnPostNetworkDataReceived(CBasePlayer* pPlayer)
{
	Vec3 vNewValue = *reinterpret_cast<Vec3*>(reinterpret_cast<uintptr_t>(pPlayer) + m_iOffset);
	Vec3 vOldValue = m_vOldValue;

	if (IsInTolerance(&vOldValue, &vNewValue))
		*reinterpret_cast<Vec3*>(reinterpret_cast<uintptr_t>(pPlayer) + m_iOffset) = vOldValue;
}

bool VectorNetvar::IsInTolerance(void* pOldValue, void* pNewValue)
{
	const Vec3 vOld = *reinterpret_cast<Vec3*>(pOldValue);
	const Vec3 vNew = *reinterpret_cast<Vec3*>(pNewValue);

	return fabsf(vNew.x - vOld.x) <= m_flTolerance
		&& fabsf(vNew.y - vOld.y) <= m_flTolerance
		&& fabsf(vNew.z - vOld.z) <= m_flTolerance;
}

void VectorNetvar::Reset()
{
	m_vOldValue = {};
}
