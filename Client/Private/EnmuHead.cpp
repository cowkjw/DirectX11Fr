#include "EnmuHead.h"
#include "GameInstance.h"
#include "EffectManager.h"
#include <BaseCharacter.h>
#include "BodyColliderParts.h"

using AniCon = CAnimController::Condition;
CEnmuHead::CEnmuHead(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnmuParts(pDevice, pContext)
{
	
}
CEnmuHead::CEnmuHead(const CEnmuHead& Prototype)
	: CEnmuParts(Prototype)
{
}

HRESULT CEnmuHead::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 20.f;
	Desc.strName = TEXT("EnmuHead");
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	Ready_Animation();

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f,-18.f,-15.f,1.f));
	m_pTransformCom->Scaling(_float3(0.1f, 0.1f, 0.1f));
    return S_OK;
}

void CEnmuHead::Priority_Update(_float fTimeDelta)
{
}

void CEnmuHead::Update(_float fTimeDelta)
{
	CEnmuParts::Update(fTimeDelta);

}

void CEnmuHead::Late_Update(_float fTimeDelta)
{
	CEnmuParts::Late_Update(fTimeDelta);
	if (m_pBodyCollider)
	{
		m_pBodyCollider->Late_Update(fTimeDelta);
	}
}

HRESULT CEnmuHead::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CEnmuHead::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
		return E_FAIL;
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_EnmuHead"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_AnimController*/
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Animator"),
		TEXT("Com_Animator"), reinterpret_cast<CComponent**>(&m_pAnimatorCom), m_pModelCom)))
		return E_FAIL;

	Ready_Animation();
	Ready_Collider();
	return S_OK;
}

void CEnmuHead::Ready_Animation()
{
	m_pAnimatorCom->PlayClip(m_pModelCom->GetAnimationClipByName("A_P1011_V23_C00_BaseNut01_1"));

	auto idleAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V23_C00_BaseNut01_1");
	idleAnim->SetLoop(true);
	auto ctrl = m_pAnimatorCom->GetAnimController();
	size_t idleIdx = ctrl->AddState("Idle", idleAnim, m_pModelCom->GetAnimationMap()[idleAnim->Get_Name()]);

	vector<CAnimation*> hurtAnims;
	for (int i = 0; i < 3; i++)
	{
		auto hurtAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V23_C00_DmgDown01_"+to_string(i));
		hurtAnim->SetLoop(false);
		hurtAnims.push_back(hurtAnim);
	}
	size_t hurtIdx = ctrl->AddState("Hurt", hurtAnims[0], m_pModelCom->GetAnimationMap()[hurtAnims[0]->Get_Name()]);
	size_t hurtIdx1 = ctrl->AddState("Hurt1", hurtAnims[1], m_pModelCom->GetAnimationMap()[hurtAnims[1]->Get_Name()]);
	size_t hurtIdx2 = ctrl->AddState("Hurt2", hurtAnims[2], m_pModelCom->GetAnimationMap()[hurtAnims[2]->Get_Name()]);

	// 상태 등록
	m_pAnimatorCom->AddTrigger("Hurt");
	m_pAnimatorCom->AddTrigger("Death");

	// 기본으로 끝나고 애니메이션 처리
	AniCon cFinished{ "", CAnimController::EOp::Finished, 0.f };

	// 상태 전환
	AniCon cHurt{ "Hurt", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, hurtIdx, cHurt, 0.1f);
	ctrl->AddTransition(hurtIdx, hurtIdx1, cFinished, 0.1f);
	ctrl->AddTransition(hurtIdx1, hurtIdx2, cFinished, 0.1f);
	ctrl->AddTransition(hurtIdx2, idleIdx, cFinished, 0.1f); // 다 끝나고 돌아오기
	ctrl->AddTransition(hurtIdx1, hurtIdx, cHurt, 0.1f);
	ctrl->AddTransition(hurtIdx2, hurtIdx, cHurt, 0.1f);

	AniCon CDeath{ "Death", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, hurtIdx, CDeath, 0.1f); // 죽으면 바로 다가감
	ctrl->AddTransition(hurtIdx, hurtIdx1, cFinished, 0.1f);
	ctrl->AddTransition(hurtIdx1, hurtIdx2, cFinished, 0.1f);

}

void CEnmuHead::Ready_Collider()
{
	//if (FAILED(Add_Component(TEXT("Com_Collider"), CSphereCollider::Create(m_pDevice, m_pContext, 5.f), reinterpret_cast<CComponent**>(&m_pColliderCom))))
	//	return;
	//m_pColliderCom->Initialize(nullptr);
	//m_pColliderCom->SetColliderType(CCollider::ColliderType::HURTBOX);
	//m_pColliderCom->SetOffset(_float3(0.f, 120.f, 108.f));
	//m_pColliderCom->SetListener(this); // 콜라이더 리스너 등록
	//m_pColliderCom->SetPriority(5);
	//
	CBodyColliderParts::BODYCOLLIDERPARTS_DESC desc{};
	desc.vColliderOffsets.push_back(_float3(0.f, 0.f, 0.f));
	desc.fRadius = 27.f;
	
		m_pBodyCollider = CBodyColliderParts::Create(m_pDevice, m_pContext);
		m_pBodyCollider->Set_BoneSocket(m_pModelCom->Get_Bone("C_Head_1"));

	this->AddChild(m_pBodyCollider);
	m_pBodyCollider->Initialize(&desc);
	m_pBodyCollider->GetCollider(0)->SetColliderType(ColliderType::SPHERE);
	m_pBodyCollider->GetCollider(0)->SetPriority(5);
	m_pBodyCollider->GetCollider(0)->SetListener(this);
	DeactiveCollider(); // 초기에는 비활성화
	//ActiveCollider();
	// C_Head_1
}

void CEnmuHead::ActiveCollider()
{
	if (m_pBodyCollider)
	{
		m_pBodyCollider->SetActive(true);
	}
}

void CEnmuHead::DeactiveCollider()
{
	if (m_pBodyCollider)
	{
		m_pBodyCollider->SetActive(false);
	}
}

CEnmuHead* CEnmuHead::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CEnmuHead* pInstance = new CEnmuHead(pDevice, pContext);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CEnmuHead");
		Safe_Release(pInstance);
	}
	return pInstance;
}

// 딱히 클론할 필요가 없음
CGameObject* CEnmuHead::Clone(void* pArg)
{
	return nullptr;
}

void CEnmuHead::Free()
{
	CEnmuParts::Free();
}

void CEnmuHead::OnCollisionEnter(CCollider* other, const XMFLOAT3& hitPos)
{
	if (other->GetType() == ColliderType::HITBOX)
	{
		CEffectManager::Get_Instance()->SpawnParticleEffect(TEXT("TanjiroHitParticle"), hitPos);
	}

}

void CEnmuHead::OnCollisionEnter(CCollider* other)
{
	if (other->GetType() == ColliderType::HITBOX)
	{
		if (auto pChar = dynamic_cast<CBaseCharacter*>(other->GetOwner()))
		{
			pChar->OnAttackHit(this);
		}
	}
	else if (other->GetType() == ColliderType::RANGE)
	{
		if (auto pAttacker = dynamic_cast<CBaseCharacter*>(other->GetOwner()))
		{
			pAttacker->OnAttackHit(m_pParent);
		}
	}
}

void CEnmuHead::OnCollisionStay(CCollider* other, float fTimeDelta)
{
}

void CEnmuHead::OnCollisionExit(CCollider* other)
{
}
