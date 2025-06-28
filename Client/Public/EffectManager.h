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
	~CEffectManager() = default;

public:
	void RegisterEffect(const _wstring& effectName, CEffect* pEffect)
	{
		if (m_EffectMap.find(effectName) != m_EffectMap.end())
		{
			return;
		}
		m_EffectMap[effectName] = pEffect;
	}

	void RemoveEffect(const _wstring& effectName)
	{
		auto it = m_EffectMap.find(effectName);
		if (it != m_EffectMap.end())
		{
			Safe_Release(it->second);
			m_EffectMap.erase(it);
		}
	}

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
	unordered_map<_wstring, class CEffect*> m_EffectMap;

private:
	virtual void Free() override;
};
END_NAMESPACE

