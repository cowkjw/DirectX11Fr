#include "EffectManager.h"

IMPLEMENT_SINGLETON(CEffectManager);

void CEffectManager::Update_ActivedParticle(_float fTimeDelta)
{

	for (auto it = m_ActiveParticleList.begin(); it != m_ActiveParticleList.end();)
	{
		CParticleEffect* pParticleEffect = *it;
		if (pParticleEffect->IsActive() == false)
		{
			it = m_ActiveParticleList.erase(it);
			m_PendingParticleList.push_back(pParticleEffect);
		}
		else
		{
			pParticleEffect->Update(fTimeDelta);
			pParticleEffect->Late_Update(fTimeDelta);
			++it;
		}
	}
}

void CEffectManager::SpawnParticleEffect(const _wstring& effectName, const _float3& position, const _float3& scale)
{
	auto it = m_EffectMap.find(effectName);
	if (it != m_EffectMap.end())
	{
		if (auto pParticleEffect = dynamic_cast<CParticleEffect*>(it->second))
		{
			m_ActiveParticleList.push_back(static_cast<CParticleEffect*>(pParticleEffect->Clone(nullptr)));

			if (m_ActiveParticleList.back()->IsActive())
			{
				m_ActiveParticleList.back()->GetTransform()->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&position), 1.f));
				m_ActiveParticleList.back()->SpwanParticle();
			}
		}
	}
}

void CEffectManager::RegisterEffect(const _wstring& effectName, CEffect* pEffect)
{
	if (m_EffectMap.find(effectName) != m_EffectMap.end())
	{
		Safe_Release(pEffect);
		return;
	}
	m_EffectMap[effectName] = pEffect;
	Safe_AddRef(pEffect);
}

void CEffectManager::ClenUpPendingParticleEffects()
{
	for (auto it = m_PendingParticleList.begin(); it != m_PendingParticleList.end();)
	{
		if ((*it)->IsActive() == false)
		{
			Safe_Release(*it);
			it = m_PendingParticleList.erase(it);
		}
		else
		{
			++it;
		}
	}
	m_PendingParticleList.clear();

	for (auto it = m_PendingMeshEffectList.begin(); it != m_PendingMeshEffectList.end();)
	{
		if ((*it)->IsActive() == false)
		{
			Safe_Release(*it);
			it = m_PendingMeshEffectList.erase(it);
		}
		else
		{
			++it;
		}
	}
	m_PendingMeshEffectList.clear();
}

void CEffectManager::RemoveEffect(const _wstring& effectName)
{
	auto it = m_EffectMap.find(effectName);
	if (it != m_EffectMap.end())
	{
		Safe_Release(it->second);
		m_EffectMap.erase(it);
	}
}

void CEffectManager::Free()
{
	__super::Free();
	for (auto& pair : m_EffectMap)
	{
		Safe_Release(pair.second);
	}
	m_EffectMap.clear();

	for (auto& particle : m_ActiveParticleList)
	{
		Safe_Release(particle);
	}
	m_ActiveParticleList.clear();
	for (auto& particle : m_PendingParticleList)
	{
		Safe_Release(particle);
	}
	m_PendingParticleList.clear();

}
