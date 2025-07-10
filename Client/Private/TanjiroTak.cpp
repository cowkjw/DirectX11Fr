#include "TanjiroTak.h"
#include "GameInstance.h"
#include "BaseCharacter.h"

CTanjiroTak::CTanjiroTak(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_fElpasedTime{ 0.f }
	, m_fDuration{ 0.f }
{
}
CTanjiroTak::CTanjiroTak(const CTanjiroTak& Prototype)
	: CGameObject(Prototype)

	, m_fElpasedTime{ Prototype.m_fElpasedTime }
	, m_fDuration{ Prototype.m_fDuration }

{
	
}

HRESULT CTanjiroTak::Initialize_Prototype()
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

HRESULT CTanjiroTak::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 30.f;
	Desc.strName = TEXT("TanjiroMig");
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;
    return S_OK;
}

void CTanjiroTak::Update(_float fTimeDelta)
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
	/*	if (m_pMigEffect && m_pMigEffect->IsActive())
		{
			m_pMigEffect->Update(fTimeDelta);
		}
		if (m_pMigRingEffect && m_pMigRingEffect->IsActive())
		{
			m_pMigRingEffect->Update(fTimeDelta);
		}*/

		if(m_pParent)
			RotationDirection(XMVector4Normalize(m_pParent->GetTransform()->Get_State(STATE::LOOK)));
	}
}

void CTanjiroTak::Late_Update(_float fTimeDelta)
{
	//if (m_pMigEffect && m_pMigEffect->IsActive())
	//{
	//	m_pMigEffect->Late_Update(fTimeDelta);
	//}
	//if (m_pMigRingEffect && m_pMigRingEffect->IsActive())
	//{
	//	m_pMigRingEffect->Late_Update(fTimeDelta);
	//}
}

void CTanjiroTak::RotationDirection(_fvector vDir)
{
	m_pTransformCom->RotateToDirection(vDir);

	//if (m_pMigEffect && m_pMigEffect->IsActive())
	//{
	//	m_pMigEffect->GetTransform()->RotateToDirection(vDir);
	//}
	//if (m_pMigRingEffect && m_pMigRingEffect->IsActive())
	//{
	//	m_pMigRingEffect->GetTransform()->RotateToDirection(vDir);
	//}

}

void CTanjiroTak::SetPosition(_fvector vPos)
{
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
	//if (m_pMigEffect && m_pMigEffect->IsActive())
	//{
	//	m_pMigEffect->GetTransform()->Set_State(STATE::POSITION, vPos);
	//}
	//if (m_pMigRingEffect && m_pMigRingEffect->IsActive())
	//{
	//	m_pMigRingEffect->GetTransform()->Set_State(STATE::POSITION, vPos);
	//}
}

void CTanjiroTak::OnEnable()
{
	//if (m_pMigRingEffect)
	//{
	//	m_pMigRingEffect->SetActive(true);
	//}
	//if (m_pMigEffect)
	//{
	//	m_pMigEffect->SetActive(true);
	//}
}

void CTanjiroTak::OnDisable()
{

	//if (m_pMigRingEffect)
	//{
	//	m_pMigRingEffect->SetActive(false);
	//}
	//if (m_pMigEffect)
	//{
	//	m_pMigEffect->SetActive(false);
	//}
}

void CTanjiroTak::OnCollisionEnter(CCollider* other)
{

}

void CTanjiroTak::OnCollisionEnter(CCollider* other, const _float3& hitPos)
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

void CTanjiroTak::OnCollisionStay(CCollider* other, float fTimeDelta)
{
}

void CTanjiroTak::OnCollisionExit(CCollider* other)
{
}

HRESULT CTanjiroTak::Ready_Effects()
{


	
	return S_OK;
}



CTanjiroTak* CTanjiroTak::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTanjiroTak* pInstance = new CTanjiroTak(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTanjiroTak");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTanjiroTak::Clone(void* pArg)
{
	CTanjiroTak* pInstance = new CTanjiroTak(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CTanjiroTak");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTanjiroTak::Free()
{
	__super::Free();

}



