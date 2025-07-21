#include "BodyColliderParts.h"
#include <BaseCharacter.h>
#include "GameInstance.h"
#include <EnmuParts.h>
#include "EffectManager.h"

CBodyColliderParts::CBodyColliderParts(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)

{
}


CBodyColliderParts::CBodyColliderParts(const CBodyColliderParts& Prototype) :
	CGameObject(Prototype)
	, m_pColliderComs{ Prototype.m_pColliderComs }
{
}

HRESULT CBodyColliderParts::Initialize_Prototype()
{
	Ready_Components();
	return S_OK;
}

HRESULT CBodyColliderParts::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};

	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 0.f;
	Desc.strName = TEXT("Body_Parts");

	if (pArg != nullptr)
	{
		BODYCOLLIDERPARTS_DESC* pDesc = reinterpret_cast<BODYCOLLIDERPARTS_DESC*>(pArg);
		auto vecOffsets =  pDesc->vColliderOffsets;
		m_eDefaultType = pDesc->eDefaultType;
		m_pColliderComs.reserve(vecOffsets.size());

		for (size_t i = 0;i<vecOffsets.size();i++)
		{
			auto offset = vecOffsets[i];
			auto pCollider = CSphereCollider::Create(m_pDevice, m_pContext, pDesc->fRadius);
			pCollider->SetOffset(offset);
			pCollider->Initialize(nullptr);
			pCollider->SetListener(this);
			pCollider->SetColliderType(pDesc->eDefaultType);
			pCollider->SetActive(false); // 초기에는 비활성화
			m_pColliderComs.push_back(pCollider);
			Add_Component(TEXT("Com_Collider" + to_wstring(i)), pCollider, reinterpret_cast<CComponent**>(&m_pColliderComs.back()));
		}
	}

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(0.3f, 0.3f, 0.3f));

	return S_OK;
}

void CBodyColliderParts::Priority_Update(_float fTimeDelta)
{
}

void CBodyColliderParts::Update(_float fTimeDelta)
{
}

void CBodyColliderParts::Late_Update(_float fTimeDelta)
{
	if (m_pBoneSocket)
	{
		_float4x4 parentWorld = m_pParent->GetTransform()->Get_WorldMatrix();
		_float4x4 boneLocal = *m_pBoneSocket->Get_CombinedTransformationMatrix();
		_matrix wepaonLocal = m_pTransformCom->Get_WorldMatrix_Inverse();
		_matrix world = XMMatrixMultiply(XMLoadFloat4x4(&boneLocal), XMLoadFloat4x4(&parentWorld));
		_matrix weaponWorld = XMMatrixMultiply(wepaonLocal, world);
		//_matrix World = XMMatrixMultiply(XMLoadFloat4x4(&m_pTransformCom->Get_WorldMatrix()),XMLoadFloat4x4(m_pBoneSocket->Get_CombinedTransformationMatrix()) );
		_float4x4 WorldMatrix{};
		XMStoreFloat4x4(&WorldMatrix, world);
		m_pTransformCom->Set_WorldMatrix(WorldMatrix);
		//	m_pTransformCom->Set_WorldMatrix();

	}

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
}

HRESULT CBodyColliderParts::Render()
{

	return S_OK;
}

void CBodyColliderParts::OnEnable()
{
	if (m_pColliderComs.empty())
		return;
	for (auto& pCollider : m_pColliderComs)
	{
		pCollider->SetActive(true); 
		pCollider->SetDrawDebug(true);
	}
}

void CBodyColliderParts::OnDisable()
{
	if (m_pColliderComs.empty())
		return;
	_bool bIsHitBox = false;
	for (auto& pCollider : m_pColliderComs)
	{
		pCollider->SetActive(false); // 초기에는 비활성화
		pCollider->SetDrawDebug(false);

		if (pCollider->GetType() == ColliderType::HITBOX)
		{
			bIsHitBox = true;
		}
	}

	if(bIsHitBox)
		 m_DamagedTargets.clear();
}

void CBodyColliderParts::Set_Radius(_uint iIndex, _float fRadius)
{
	m_pColliderComs[iIndex]->SetRadius(fRadius);
}



HRESULT CBodyColliderParts::Ready_Components()
{

	return S_OK;
}


CBodyColliderParts* CBodyColliderParts::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBodyColliderParts* pInstance = new CBodyColliderParts(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CBodyColliderParts");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CBodyColliderParts::Clone(void* pArg)
{
	CBodyColliderParts* pInstance = new CBodyColliderParts(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CBodyColliderParts");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBodyColliderParts::Free()
{
	__super::Free();

	for (auto& pCollider : m_pColliderComs)
	{
		Safe_Release(pCollider);
	}
	m_pColliderComs.clear();

}

void CBodyColliderParts::OnCollisionEnter(CCollider* other)
{

	if (auto pTarget = dynamic_cast<CBaseCharacter*>(other->GetOwner()))
	{
		if (pTarget == m_pParent)
			return;
		if (m_DamagedTargets.find(pTarget) != m_DamagedTargets.end())
			return; // 이미 데미지를 입힌 대상이면 무시
		m_DamagedTargets.insert(pTarget); // 데미지를 입힌 대상에 추가

		if (auto pChar = dynamic_cast<CBaseCharacter*>(m_pParent))
		{
			if (pTarget->GetState() != CBaseCharacter::CSTATE::DIE)
			{
				pChar->OnAttackHit(pTarget);
				for (auto& pCollider : m_pColliderComs)
				{
					if (pCollider->GetType() == ColliderType::HITBOX)
					{
						pCollider->SetActive(false);
						pCollider->SetDrawDebug(false);
					}
				}
			}
		}
		else if (auto pBossParts = dynamic_cast<CEnmuParts*>(m_pParent))
		{
			auto pBoss =static_cast<CEnmuMeat*>(pBossParts->GetParent());
			if (pBoss)
			{
				pBoss->OnAttackHit(pTarget);
			}
		}
	}
}

void CBodyColliderParts::OnCollisionEnter(CCollider* other, const _float3& hitPos)
{
	auto pMyParent = m_pParent;
	auto otherOwner = other->GetOwner();
	if (pMyParent == otherOwner->GetParent())
		return; // 자기 자신과 충돌은 무시)

	if (other->GetType() != ColliderType::HITBOX&&other->GetType()!=ColliderType::ENVIRONMENT)
	{
		if (m_DamagedTargets.find(other->GetOwner()->GetParent()) != m_DamagedTargets.end())
			return; // 이미 데미지를 입힌 대상이면 무시
		m_DamagedTargets.insert(other->GetOwner()->GetParent()); // 데미지를 입힌 대상에 추가
		for (auto& pCollider : m_pColliderComs)
		{
			if (pCollider->GetType() == ColliderType::HITBOX)
			{
				pCollider->SetActive(false);
				pCollider->SetDrawDebug(false);
			}
		}
		if (dynamic_cast<CBaseCharacter*>(otherOwner))
		{
			CEffectManager::Get_Instance()->SpawnParticleEffect(TEXT("HitBodyShockParticle"), hitPos);
			CSoundMag::Get_Instance()->PlayEffect("event:/Common/BodyAttack");
		}
		
	}
}

void CBodyColliderParts::OnCollisionStay(CCollider* other, float fTimeDelta)
{
	if (auto pTarget = dynamic_cast<CBaseCharacter*>(other->GetOwner()))
	{
		if (m_DamagedTargets.find(pTarget) != m_DamagedTargets.end())
			return; // 이미 데미지를 입힌 대상이면 무시
		m_DamagedTargets.insert(pTarget); // 데미지를 입힌 대상에 추가

		if (auto pChar = dynamic_cast<CBaseCharacter*>(m_pParent))
		{
			if (pTarget->GetState() != CBaseCharacter::CSTATE::DIE)
			{
				pChar->OnAttackHit(pTarget);
				for (auto& pCollider : m_pColliderComs)
				{
					if (pCollider->GetType() == ColliderType::HITBOX)
					{
						pCollider->SetActive(false);
						pCollider->SetDrawDebug(false);
					}
				}
			}
		}
		else if (auto pBossParts = dynamic_cast<CEnmuParts*>(m_pParent))
		{
			auto pBoss = static_cast<CEnmuMeat*>(pBossParts->GetParent());
			if (pBoss)
			{
				CSoundMag::Get_Instance()->PlayEffect("event:/Enmu/Hited");
				pBoss->OnAttackHit(pTarget);
			}
		}
	}
}

void CBodyColliderParts::OnCollisionExit(CCollider* other)
{
	
}
