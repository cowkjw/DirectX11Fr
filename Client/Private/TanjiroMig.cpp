#include "TanjiroMig.h"
#include "GameInstance.h"
#include "BaseCharacter.h"
#include "EffectManager.h"

CTanjiroMig::CTanjiroMig(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_fElpasedTime{ 0.f }
	, m_fDuration{ 0.f }
{
}
CTanjiroMig::CTanjiroMig(const CTanjiroMig& Prototype)
	: CGameObject(Prototype)
	, m_pMigEffect{ Prototype.m_pMigEffect }
	, m_pMigRingEffect{ Prototype.m_pMigRingEffect }
	, m_fElpasedTime{ Prototype.m_fElpasedTime }
	, m_fDuration{ Prototype.m_fDuration }

{
		Safe_AddRef(m_pMigEffect);
		Safe_AddRef(m_pMigRingEffect);
}

HRESULT CTanjiroMig::Initialize_Prototype()
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 30.f;
	Desc.strName = TEXT("TanjiroMig");
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if(FAILED(Ready_Effects()))
		return E_FAIL;
	m_fDuration = 1.f;
	return S_OK;
}

HRESULT CTanjiroMig::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 30.f;
	Desc.strName = TEXT("TanjiroMig");
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;
    return S_OK;
}

void CTanjiroMig::Update(_float fTimeDelta)
{
	if (m_fElpasedTime >= m_fDuration)
	{
		SetActive(false);
		m_fElpasedTime = 0.f;
		return;
	}
	else
	{
		m_pTransformCom->MoveDirection(m_pTransformCom->Get_State(STATE::LOOK), fTimeDelta);
		m_fElpasedTime += fTimeDelta;
		if (m_pMigEffect && m_pMigEffect->IsActive())
		{
			m_pMigEffect->Update(fTimeDelta);
		}
		if (m_pMigRingEffect && m_pMigRingEffect->IsActive())
		{
			m_pMigRingEffect->Update(fTimeDelta);
		}

		if(m_pParent)
			RotationDirection(XMVector4Normalize(m_pParent->GetTransform()->Get_State(STATE::LOOK)));
	}


	_float t = m_fElpasedTime / m_fDuration;
	t = (t > 1.f) ? 1.f : t;

	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_float3 vSpawnPos{};
	XMStoreFloat3(&vSpawnPos, vPos);

	for (int i = 0; i < PARTICLE_COUNT; ++i)
	{
		if (!m_bSpawnedParticle[i] && t >= m_fParticleTriggers[i])
		{
			CEffectManager::Get_Instance()
				->SpawnParticleEffect(L"SpreadWater", vSpawnPos, _float3(2.f, 1.5f, 1.f));
			m_bSpawnedParticle[i] = true;
		}
	}
}

void CTanjiroMig::Late_Update(_float fTimeDelta)
{
	if (m_pMigEffect && m_pMigEffect->IsActive())
	{
		m_pMigEffect->Late_Update(fTimeDelta);
	}
	if (m_pMigRingEffect && m_pMigRingEffect->IsActive())
	{
		m_pMigRingEffect->Late_Update(fTimeDelta);
	}
}

void CTanjiroMig::RotationDirection(_fvector vDir)
{
	m_pTransformCom->RotateToDirection(vDir);

	if (m_pMigEffect && m_pMigEffect->IsActive())
	{
		m_pMigEffect->GetTransform()->RotateToDirection(vDir);
	}
	if (m_pMigRingEffect && m_pMigRingEffect->IsActive())
	{
		m_pMigRingEffect->GetTransform()->RotateToDirection(vDir);
	}

}

void CTanjiroMig::SetPosition(_fvector vPos)
{
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
	if (m_pMigEffect && m_pMigEffect->IsActive())
	{
		m_pMigEffect->GetTransform()->Set_State(STATE::POSITION, vPos);
	}
	if (m_pMigRingEffect && m_pMigRingEffect->IsActive())
	{
		m_pMigRingEffect->GetTransform()->Set_State(STATE::POSITION, vPos);
	}
}

void CTanjiroMig::OnEnable()
{
	if (m_pMigRingEffect)
	{
		m_pMigRingEffect->SetActive(true);
	}
	if (m_pMigEffect)
	{
		m_pMigEffect->SetActive(true);
	}
}

void CTanjiroMig::OnDisable()
{

	if (m_pMigRingEffect)
	{
		m_pMigRingEffect->SetActive(false);
	}
	if (m_pMigEffect)
	{
		m_pMigEffect->SetActive(false);
	}
	for (int i = 0; i < PARTICLE_COUNT; ++i)
	{
		m_bSpawnedParticle[i] = false;
	}
}

void CTanjiroMig::OnCollisionEnter(CCollider* other)
{

}

void CTanjiroMig::OnCollisionEnter(CCollider* other, const _float3& hitPos)
{
	if (other->GetOwner() == m_pParent)
		return;
	if (other->GetType() != ColliderType::HITBOX)
	{
		auto pTarget = other->GetOwner();
		if (auto pChar = dynamic_cast<CBaseCharacter*>(pTarget))
		{
			pChar->Blow(this->m_pParent, 30.f);
			pChar->TakeDamage(10.f);
			pChar->TakeDamage(5.f);
			pChar->TakeDamage(3.5f);
			pChar->StartHitStop(0.35f);
			dynamic_cast<CBaseCharacter*>(m_pParent)->StartHitStop(0.35f);
		}
	}
}

void CTanjiroMig::OnCollisionStay(CCollider* other, float fTimeDelta)
{
}

void CTanjiroMig::OnCollisionExit(CCollider* other)
{
}

HRESULT CTanjiroMig::Ready_Effects()
{

	m_pMigEffect = CTanMigEffect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pMigEffect)
		return E_FAIL;
	m_pMigEffect->Initialize(nullptr);
	m_pMigEffect->SetActive(false);


	m_pMigRingEffect = CTanMigRingEffect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pMigRingEffect)
		return E_FAIL;
	m_pMigRingEffect->Initialize(nullptr);
	m_pMigRingEffect->SetActive(false);

	return S_OK;
}



CTanjiroMig* CTanjiroMig::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTanjiroMig* pInstance = new CTanjiroMig(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTanjiroMig");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTanjiroMig::Clone(void* pArg)
{
	CTanjiroMig* pInstance = new CTanjiroMig(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CTanjiroMig");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTanjiroMig::Free()
{
	__super::Free();
	Safe_Release(m_pMigEffect);
	Safe_Release(m_pMigRingEffect);
}



