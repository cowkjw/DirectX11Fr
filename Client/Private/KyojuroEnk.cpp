#include "KyojuroEnk.h"
#include "GameInstance.h"
#include "BaseCharacter.h"

CKyojuroEnk::CKyojuroEnk(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_pEnkEffect{ nullptr }
	, m_fElpasedTime{ 0.f }
	, m_fDuration{ 0.f }
{
}
CKyojuroEnk::CKyojuroEnk(const CKyojuroEnk& Prototype)
	: CGameObject(Prototype)
	, m_SpiralEffects{ Prototype.m_SpiralEffects }
	, m_pEnkEffect{ Prototype.m_pEnkEffect }
	, m_Effects{ Prototype.m_Effects }
	, m_fElpasedTime{ Prototype.m_fElpasedTime }
	, m_fDuration{ Prototype.m_fDuration }
	, m_pColliderCom{ Prototype.m_pColliderCom }
{
	for (auto& pSpiral : m_SpiralEffects)
	{
		Safe_AddRef(pSpiral);
	}
}

HRESULT CKyojuroEnk::Initialize_Prototype()
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 40.f;
	Desc.strName = TEXT("KyojuroEnk");
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if(FAILED(Ready_Effects()))
		return E_FAIL;
	m_fDuration = 0.45f;
	Add_Component(TEXT("Com_BoxCollider"), CBoxCollider::Create(m_pDevice, m_pContext, _float3(10.f, 10.f, 20.f)), reinterpret_cast<CComponent**>(&m_pColliderCom));
	m_pColliderCom->Initialize(nullptr);
	m_pColliderCom->SetOffset(_float3(0.f, 0.f, -20.f));
	m_pColliderCom->SetListener(this);
	m_pColliderCom->SetActive(false);
	m_pColliderCom->SetDrawDebug(true);
	m_pColliderCom->SetColliderType(ColliderType::HITBOX);
	m_pColliderCom->Update();
	return S_OK;
}

HRESULT CKyojuroEnk::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 40.f;
	Desc.strName = TEXT("KyojuroEnk");
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;
    return S_OK;
}

void CKyojuroEnk::Update(_float fTimeDelta)
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
		for (auto& pSpiral : m_SpiralEffects)
		{
			if (pSpiral && pSpiral->IsActive())
			{
				pSpiral->GetTransform()->MoveDirection(m_pTransformCom->Get_State(STATE::LOOK), fTimeDelta);
			}
		}
		//if (m_pEnkEffect && m_pEnkEffect->IsActive())
		//{
		//	m_pEnkEffect->GetTransform()->MoveDirection(m_pTransformCom->Get_State(STATE::LOOK), fTimeDelta);
		//}

		for (const auto& pSpiral : m_Effects)
		{
			if (pSpiral && pSpiral->IsActive())
			{
				pSpiral->GetTransform()->MoveDirection(m_pTransformCom->Get_State(STATE::LOOK), fTimeDelta);
		
			}
		}
	}
	for (const auto& pSpiral : m_SpiralEffects)
	{
		if (pSpiral && pSpiral->IsActive())
		{
			pSpiral->Update(fTimeDelta);
		}
	}
	//if (m_pEnkEffect && m_pEnkEffect->IsActive())
	//{
	//	m_pEnkEffect->Update(fTimeDelta);
	//}

	for (const auto& pSpiral : m_Effects)
	{
		if (pSpiral && pSpiral->IsActive())
		{
			pSpiral->Update(fTimeDelta);
		}
	}
}

void CKyojuroEnk::Late_Update(_float fTimeDelta)
{
	for (const auto& pSpiral : m_SpiralEffects)
	{
		if (pSpiral&&pSpiral->IsActive())
		{
			pSpiral->Late_Update(fTimeDelta);
		}
	}
	//if (m_pEnkEffect && m_pEnkEffect->IsActive())
	//{
	//	m_pEnkEffect->Late_Update(fTimeDelta);
	//}
	for (const auto& pSpiral : m_Effects)
	{
		if (pSpiral && pSpiral->IsActive())
		{
			pSpiral->Late_Update(fTimeDelta);
		}
	}
}

void CKyojuroEnk::RotationDirection(_fvector vDir)
{
	m_pTransformCom->RotateToDirection(vDir);
	for (auto& pSpiral : m_SpiralEffects)
	{
		if (pSpiral && pSpiral->IsActive())
		{
			pSpiral->GetTransform()->RotateToDirection(vDir);
		}
	}

	for (auto& pSpiral : m_Effects)
	{
		if (pSpiral && pSpiral->IsActive())
		{
			pSpiral->GetTransform()->RotateToDirection(vDir);
		}
	}
	//if (m_pEnkEffect && m_pEnkEffect->IsActive())
	//{
	//	m_pEnkEffect->GetTransform()->RotateToDirection(vDir);
	//}
}

void CKyojuroEnk::SetPosition(_fvector vPos)
{
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
	for (auto& pSpiral : m_SpiralEffects)
	{
		if (pSpiral && pSpiral->IsActive())
		{
			pSpiral->GetTransform()->Set_State(STATE::POSITION, vPos);
		}
	}
	//if (m_pEnkEffect && m_pEnkEffect->IsActive())
	//{
	//	m_pEnkEffect->GetTransform()->Set_State(STATE::POSITION, vPos);
	//}
	for (auto& pSpiral : m_Effects)
	{
		if (pSpiral && pSpiral->IsActive())
		{
			pSpiral->GetTransform()->Set_State(STATE::POSITION, vPos);
		}
	}
}

void CKyojuroEnk::OnEnable()
{
	/*if (m_pEnkEffect)
	{
		m_pEnkEffect->SetActive(true);
	}*/
	for (auto& pSpiral : m_SpiralEffects)
	{
		if (pSpiral)
		{
			pSpiral->SetActive(true);
		}
	}
	for (auto& pSpiral : m_Effects)
	{
		if (pSpiral)
		{
			pSpiral->SetActive(true);
		}
	}
	if (m_pColliderCom)
	{
		m_pColliderCom->SetDrawDebug(true);
		m_pColliderCom->SetActive(true); // 콜라이더 활성화
	}
}

void CKyojuroEnk::OnDisable()
{
	/*if (m_pEnkEffect)
	{
		m_pEnkEffect->SetActive(false);
	}*/
	for (auto& pSpiral : m_SpiralEffects)
	{
		if (pSpiral)
		{
			pSpiral->SetActive(false);
		}
	}

	for (auto& pSpiral : m_Effects)
	{
		if (pSpiral)
		{
			pSpiral->SetActive(false);
		}
	}
	if (m_pColliderCom)
	{
		//m_pColliderCom->SetDrawDebug(false);
		m_pColliderCom->SetActive(false); // 콜라이더 비활성화
	}
}

void CKyojuroEnk::OnCollisionEnter(CCollider* other)
{
	int a = 0;
}

void CKyojuroEnk::OnCollisionEnter(CCollider* other, const _float3& hitPos)
{
	if (other->GetOwner() == m_pParent)
		return;
	if (other->GetType() != ColliderType::HITBOX)
	{
		auto pTarget = other->GetOwner();
		if (auto pChar = dynamic_cast<CBaseCharacter*>(pTarget))
		{
			m_pColliderCom->SetActive(false); 
			m_pColliderCom->SetDrawDebug(false); 
			pChar->Blow(this, 30.f);
			pChar->TakeDamage(10.f);
			pChar->TakeDamage(5.f);
			pChar->TakeDamage(3.5f);
			pChar->StartHitStop(0.25f);
			dynamic_cast<CBaseCharacter*>(m_pParent)->StartHitStop(0.25f);
		}
	}
}

void CKyojuroEnk::OnCollisionStay(CCollider* other, float fTimeDelta)
{
}

void CKyojuroEnk::OnCollisionExit(CCollider* other)
{
}

HRESULT CKyojuroEnk::Ready_Effects()
{

	m_pEnkEffect = CKyojuroEnkEffect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pEnkEffect)
		return E_FAIL;
	m_pEnkEffect->Initialize(nullptr);
	m_pEnkEffect->SetActive(false);

	m_SpiralEffects.reserve(4);
	for (_int i = 0; i < 4; ++i)
	{
		_float fYawOffset = XM_PIDIV2 * i;
		_float fDegree = XMConvertToDegrees(fYawOffset);
		auto pSpiral = CKyojuroEnkSpiralEffect::Create(m_pDevice, m_pContext);
		if (nullptr == pSpiral)
			return E_FAIL;
		pSpiral->Initialize(nullptr);
		pSpiral->GetTransform()->Rotate_EulerAngles(_float3(0.f,0.f, fDegree));
		pSpiral->SetDirection(i % 2 == 0 ? -1.f : 1.f); // 왼쪽(0,2)과 오른쪽(1,3) 방향 설정
		m_SpiralEffects.push_back(pSpiral);
		pSpiral->SetActive(false); 
	}

	m_Effects.reserve(3);
	for (_int i = 0; i < 1; ++i)
	{
		_float fYawOffset = XM_PIDIV2 * i;
		_float fDegree = XMConvertToDegrees(fYawOffset);
		auto pSpiral = CKyojuroEnkEffect::Create(m_pDevice, m_pContext);
		if (nullptr == pSpiral)
			return E_FAIL;
		pSpiral->Initialize(nullptr);
		pSpiral->GetTransform()->Rotate_EulerAngles(_float3(0.f, 0.f, fDegree));
		m_Effects.push_back(pSpiral);
		pSpiral->SetActive(false);
	}

	return S_OK;
}



CKyojuroEnk* CKyojuroEnk::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKyojuroEnk* pInstance = new CKyojuroEnk(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CKyojuroEnk");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CKyojuroEnk::Clone(void* pArg)
{
	CKyojuroEnk* pInstance = new CKyojuroEnk(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CKyojuroEnk");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CKyojuroEnk::Free()
{
	__super::Free();
	for (auto& pSpiral : m_SpiralEffects)
	{
		Safe_Release(pSpiral);
	}
	m_SpiralEffects.clear();
	for (auto& pEffect : m_Effects)
	{
		Safe_Release(pEffect);
	}
	m_Effects.clear();
	Safe_Release(m_pEnkEffect);
	Safe_Release(m_pColliderCom);
}



