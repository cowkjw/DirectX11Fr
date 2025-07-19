#include "TanjiroNej.h"
#include "GameInstance.h"
#include "EffectManager.h"
#include "BaseCharacter.h"
CTanjiroNej::CTanjiroNej(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_fElpasedTime{ 0.f }
	, m_fDuration{ 0.f }
{
}
CTanjiroNej::CTanjiroNej(const CTanjiroNej& Prototype)
	: CGameObject(Prototype)
	, m_pWindEffects{ Prototype.m_pWindEffects }
	, m_pVortexEffect{ Prototype.m_pVortexEffect }
	, m_fElpasedTime{ Prototype.m_fElpasedTime }
	, m_fDuration{ Prototype.m_fDuration }

{
	for (auto& effect : m_pWindEffects)
	{
		Safe_AddRef(effect);
	}

	Safe_AddRef(m_pVortexEffect);
}

HRESULT CTanjiroNej::Initialize_Prototype()
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 30.f;
	Desc.strName = TEXT("TanjiroNej");
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;
	m_fDuration = 1.5f;
	if(FAILED(Ready_Effects()))
		return E_FAIL;
	return S_OK;
}

HRESULT CTanjiroNej::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 30.f;
	Desc.strName = TEXT("TanjiroNej");
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;


	/*CParticleEffect* pEffect = CNejWaterParticle::Create(m_pDevice, m_pContext);
	if (pEffect == nullptr)
		return E_FAIL;
	pEffect->Initialize(nullptr);
	CEffectManager::Get_Instance()->RegisterEffect(TEXT("Water"), pEffect);*/
    return S_OK;
}

void CTanjiroNej::Update(_float fTimeDelta)
{
	if (m_fElpasedTime >= m_fDuration)
	{
		SetActive(false);
		m_fElpasedTime = 0.f;
		return;
	}
	else
	{
		m_fElpasedTime += fTimeDelta;

		for (const auto& pWindEffect : m_pWindEffects)
		{
			if (pWindEffect && pWindEffect->IsActive())
			{
				pWindEffect->Update(fTimeDelta);
			}
		}

		if (m_pVortexEffect && m_pVortexEffect->IsActive())
		{
			m_pVortexEffect->Update(fTimeDelta);
		}
	}

	_float t = m_fElpasedTime / m_fDuration;
	t = (t > 1.f) ? 1.f : t;

	const _float riseEnd = 0.3f;
	const _float startY = -10.f;
	const _float peakY = 15.f;
	float newY = (t < riseEnd)
		? (startY + (peakY - startY) * (t / riseEnd))
		: peakY;

	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	vPos.m128_f32[1] = newY;
	_float3 vSpawnPos{};
	XMStoreFloat3(&vSpawnPos, vPos);

	for (int i = 0; i < PARTICLE_COUNT; ++i)
	{
		if (!m_bSpawnedParticle[i] && t >= m_fParticleTriggers[i])
		{
			CEffectManager::Get_Instance()
				->SpawnParticleEffect(L"SpreadWater", vSpawnPos,_float3(3.f,1.f,1.f));
			m_bSpawnedParticle[i] = true;
		}
	}
}

void CTanjiroNej::Late_Update(_float fTimeDelta)
{
	for (const auto& pWindEffect : m_pWindEffects)
	{
		if (pWindEffect && pWindEffect->IsActive())
		{
			pWindEffect->Late_Update(fTimeDelta);
		}
	}
	if (m_pVortexEffect && m_pVortexEffect->IsActive())
	{
		m_pVortexEffect->Late_Update(fTimeDelta);
	}
}

void CTanjiroNej::RotationDirection(_fvector vDir)
{
	m_pTransformCom->RotateToDirection(vDir);

	for (const auto& pWindEffect : m_pWindEffects)
	{
		if(pWindEffect)
			pWindEffect->GetTransform()->RotateToDirection(vDir);
	}

	if (m_pVortexEffect && m_pVortexEffect->IsActive())
	{
		m_pVortexEffect->GetTransform()->RotateToDirection(vDir);
	}

}

void CTanjiroNej::SetPosition(_fvector vPos)
{
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
	for (const auto& pWindEffect : m_pWindEffects)
	{
		if (pWindEffect)
			pWindEffect->GetTransform()->Set_State(STATE::POSITION, vPos);
	}
	if (m_pVortexEffect)
	{
		m_pVortexEffect->GetTransform()->Set_State(STATE::POSITION, vPos);
	}
}

void CTanjiroNej::OnEnable()
{
	for (const auto& pWindEffect : m_pWindEffects)
	{
		if (pWindEffect)
			pWindEffect->SetActive(true);
	}
	if (m_pVortexEffect)
	{
		m_pVortexEffect->SetActive(true);
	}
}

void CTanjiroNej::OnDisable()
{

	for (const auto& pWindEffect : m_pWindEffects)
	{
		if (pWindEffect)
			pWindEffect->SetActive(false);
	}
	if (m_pVortexEffect)
	{
		m_pVortexEffect->SetActive(false);
	}

	for (int i = 0; i < PARTICLE_COUNT; ++i)
	{
		m_bSpawnedParticle[i] = false;
	}
}

void CTanjiroNej::OnCollisionEnter(CCollider* other)
{

}

void CTanjiroNej::OnCollisionEnter(CCollider* other, const _float3& hitPos)
{

}

void CTanjiroNej::OnCollisionStay(CCollider* other, float fTimeDelta)
{
}

void CTanjiroNej::OnCollisionExit(CCollider* other)
{
}

HRESULT CTanjiroNej::Ready_Effects()
{

	for (_int i = 0; i < m_pWindEffects.size(); i++)
	{
		m_pWindEffects[i] = CTanNejWindEffect::Create(m_pDevice, m_pContext);
		if (nullptr == m_pWindEffects[i])
			return E_FAIL;
		m_pWindEffects[i]->SetDirection(i);
		m_pWindEffects[i]->Initialize(nullptr);
		m_pWindEffects[i]->SetActive(false);
	}


	m_pVortexEffect = CTanNejVortexEffect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVortexEffect)
		return E_FAIL;
	m_pVortexEffect->Initialize(nullptr);
	m_pVortexEffect->SetActive(false);

	return S_OK;
}



CTanjiroNej* CTanjiroNej::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTanjiroNej* pInstance = new CTanjiroNej(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTanjiroNej");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTanjiroNej::Clone(void* pArg)
{
	CTanjiroNej* pInstance = new CTanjiroNej(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CTanjiroNej");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTanjiroNej::Free()
{
	__super::Free();
	for (auto& pEffect : m_pWindEffects)
	{
		Safe_Release(pEffect);
	}
	Safe_Release(m_pVortexEffect);
}



