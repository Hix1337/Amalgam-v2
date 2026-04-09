#pragma once
#include "../../SDK/SDK.h"

#define DIST_EPSILON 0.03125f
#define EQUAL_EPSILON 0.001f

class BaseNetvar
{
public:
	int m_iOffset = 0;
	float m_flTolerance = 0.f;

	virtual void OnPreEntityPacketReceived(CBasePlayer* pPlayer) = 0;
	virtual void OnPostNetworkDataReceived(CBasePlayer* pPlayer) = 0;
	virtual bool IsInTolerance(void* pOldValue, void* pNewValue) = 0;
	virtual void Reset() = 0;
	virtual ~BaseNetvar() = default;
};

class FloatNetvar final : public BaseNetvar
{
public:
	float m_flOldValue = 0.f;

	FloatNetvar(int iOffset, float flTolerance);
	void OnPreEntityPacketReceived(CBasePlayer* pPlayer) override;
	void OnPostNetworkDataReceived(CBasePlayer* pPlayer) override;
	bool IsInTolerance(void* pOldValue, void* pNewValue) override;
	void Reset() override;
};

class VectorNetvar final : public BaseNetvar
{
public:
	Vec3 m_vOldValue = {};

	VectorNetvar(int iOffset, float flTolerance);
	void OnPreEntityPacketReceived(CBasePlayer* pPlayer) override;
	void OnPostNetworkDataReceived(CBasePlayer* pPlayer) override;
	bool IsInTolerance(void* pOldValue, void* pNewValue) override;
	void Reset() override;
};

class CCompressionHandler
{
	std::vector<std::unique_ptr<BaseNetvar>> m_vCompressedVars;
	bool m_bReceivedPacket = false;

	// resolved RestoreData function pointer
	using RestoreDataFn = int(__fastcall*)(void*, const char*, int, int);
	RestoreDataFn m_fnRestoreData = nullptr;

public:
	void Init();
	void OnPreEntityPacketReceived(CBasePlayer* pPlayer, int iCommandSlot);
	void OnPostNetworkDataReceived(CBasePlayer* pPlayer, int iCommandsAcknowledged);
	void OnLevelInit();

	static float AssignRangeMultiplier(int nBits, double range);
	static bool CloseEnough(float a, float b, float epsilon = EQUAL_EPSILON);
};

ADD_FEATURE(CCompressionHandler, CompressionHandler);
