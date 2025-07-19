#include "TanjiroTak.h"
#include "GameInstance.h"
#include "BaseCharacter.h"
#include <EnmuParts.h>

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
	, m_pColliderCom{ Prototype.m_pColliderCom }

{
	Safe_AddRef(m_pColliderCom);
}

HRESULT CTanjiroTak::Initialize_Prototype()
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 30.f;
	Desc.strName = TEXT("TanjiroTak");
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if(FAILED(Ready_Effects()))
		return E_FAIL;
	m_fDuration = 1.f;
	m_pColliderCom = CSphereCollider::Create(m_pDevice, m_pContext, 3.f);

	if (!m_pColliderCom)
		return E_FAIL;

	Add_Component(TEXT("Com_Collider"), CSphereCollider::Create(m_pDevice, m_pContext, 3.f), reinterpret_cast<CComponent**>(&m_pColliderCom));

	m_pColliderCom->Initialize(nullptr);
	m_pColliderCom->SetListener(this);
	m_pColliderCom->SetColliderType(ColliderType::HITBOX);
	m_pColliderCom->SetActive(false);
	return S_OK;
}

HRESULT CTanjiroTak::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 30.f;
	Desc.strName = TEXT("TanjiroTak");
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
		if(m_pParent)
			RotationDirection(XMVector4Normalize(m_pParent->GetTransform()->Get_State(STATE::LOOK)));
	}
}

void CTanjiroTak::Late_Update(_float fTimeDelta)
{
}

void CTanjiroTak::RotationDirection(_fvector vDir)
{
	m_pTransformCom->RotateToDirection(vDir);

}

void CTanjiroTak::SetPosition(_fvector vPos)
{
	m_pTransformCom->Set_State(STATE::POSITION, vPos);

}

void CTanjiroTak::OnEnable()
{
	if (m_pColliderCom)
	{
		m_pColliderCom->SetActive(true);
		m_pColliderCom->SetDrawDebug(true);
	}
}

void CTanjiroTak::OnDisable()
{
	if (m_pColliderCom)
	{
		m_pColliderCom->SetActive(false);
		m_pColliderCom->SetDrawDebug(false);
	}
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
		if (auto pChar = dynamic_cast<CEnmuParts*>(pTarget))
		{
			auto pCharParent = dynamic_cast<CEnmuMeat*>(pChar->GetParent());
			if (!pCharParent)
				return;
			pCharParent->Hit(10.f);
			pCharParent->Hit(5.f);
			pCharParent->Hit(3.5f);
			dynamic_cast<CBaseCharacter*>(m_pParent)->StartHitStop(0.35f);
			m_pColliderCom->SetActive(false);
			m_pColliderCom->SetDrawDebug(false);
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
	Safe_Release(m_pColliderCom);
}



