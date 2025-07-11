#pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "MeshEffect.h"
#include "ParticleEffect.h"

BEGIN_NAMESPACE(Client)
class CEffectManager :public CBase
{
	DECLARE_SINGLETON(CEffectManager)
private:
	CEffectManager() {};
	virtual ~CEffectManager() = default;

public:
	void EnableConsole();

	void Update_ActivedParticle(_float fTimeDelta);
	void Late_Update(_float fTimeDelta);

	void SpawnParticleEffect(const _wstring& effectName, const _float3& position, const _float3& scale = _float3(1.f, 1.f, 1.f));

	void RegisterEffect(const _wstring& effectName, CEffect* pEffect);

	void ClenUpPendingParticleEffects();

	void RemoveEffect(const _wstring& effectName);
	


	CEffect* GetEffect(const _wstring& effectName)
	{
		auto it = m_EffectMap.find(effectName);
		if (it != m_EffectMap.end())
		{
			return it->second;
		}
		return nullptr;
	}
private:
	unordered_map<_wstring, CEffect*> m_EffectMap;
	list<CParticleEffect*> m_ActiveParticleList;
	list<CParticleEffect*> m_PendingParticleList;
	list <CMeshEffect*> m_ActiveMeshEffectList;
	list <CMeshEffect*> m_PendingMeshEffectList;
public:
	virtual void Free() override;
};
END_NAMESPACE

