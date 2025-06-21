#include "EnmuBody.h"
#include "JsonLoader.h"	
#include "GameInstance.h"


using AniCon = CAnimController::Condition;

CEnmuBody::CEnmuBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnmuParts(pDevice, pContext)
{
}
CEnmuBody::CEnmuBody(const CEnmuBody& Prototype)
	: CEnmuParts(Prototype)
{
}

HRESULT CEnmuBody::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 20.f;
	Desc.strName = TEXT("EnmuBody");
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pAnimatorCom->RegisterEventListener("ActiveHitbox", [&](const string&) {

		ActiveCollider();
		});
	m_pAnimatorCom->RegisterEventListener("DeactiveHitbox", [&](const string&) {
		DeactiveCollider();
		});

	Ready_Animation();

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, -20.f, 0.f, 1.f));


    return S_OK;
}

void CEnmuBody::Priority_Update(_float fTimeDelta)
{
}

void CEnmuBody::Update(_float fTimeDelta)
{
	CEnmuParts::Update(fTimeDelta);
	if (m_eState == EnmuState::OPEN)
	{
		m_pColliderCom->SetActive(false);
	}
	else
	{
		m_pColliderCom->SetActive(true);
	}
}

void CEnmuBody::Late_Update(_float fTimeDelta)
{
	CEnmuParts::Late_Update(fTimeDelta);
}

HRESULT CEnmuBody::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CEnmuBody::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
		return E_FAIL;
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_Body"),
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

void CEnmuBody::Ready_Animation()
{
	m_pAnimatorCom->PlayClip(m_pModelCom->GetAnimationClipByName("A_P1011_V17_C00_BaseNut01_1"));

	auto animations = m_pModelCom->GetAnimations();
	CJsonLoader jsonLoader;
	jsonLoader.LoadAnimEvent("../Asset/Json/EnmuBody_events.json", animations);
	jsonLoader.Free();

	auto ctrl = m_pAnimatorCom->GetAnimController();

	auto idleAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V17_C00_BaseNut01_1");
	idleAnim->SetLoop(true);
	size_t idleIdx = ctrl->AddState("Idle", idleAnim, m_pModelCom->GetAnimationMap()[idleAnim->Get_Name()]);

	auto hurtAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V17_C00_DmgDownS01_Add");
	hurtAnim->SetLoop(false);
	size_t damgeIdx = ctrl->AddState("Hurt", hurtAnim, m_pModelCom->GetAnimationMap()[hurtAnim->Get_Name()]);

	// 특정 피에 열리는 0은 열리는거 1은 그 상태에서 Loop 2는 다시 닫히는거
	vector<CAnimation*> openAnims;
	for (_int i = 0; i < 3; i++)
	{
		auto openAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V17_C00_DmgBreak01_" + to_string(i));
		openAnim->SetLoop(false);
		openAnims.push_back(openAnim);
	}
	openAnims[1]->SetLoop(true); // 1번은 루프
	size_t openIdx = ctrl->AddState("Open0", openAnims[0], m_pModelCom->GetAnimationMap()[openAnims[0]->Get_Name()]);
	size_t openIdx1 = ctrl->AddState("Open1", openAnims[1], m_pModelCom->GetAnimationMap()[openAnims[1]->Get_Name()]);
	size_t openIdx2 = ctrl->AddState("Open2", openAnims[2], m_pModelCom->GetAnimationMap()[openAnims[2]->Get_Name()]);



	// 스킬 8에 진행되는
	auto freezAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V17_C00_AtkSkl08");
	freezAnim->SetLoop(false);
	size_t freezIdx = ctrl->AddState("FreezAttack", freezAnim, m_pModelCom->GetAnimationMap()[freezAnim->Get_Name()]);

	auto angryAnimCut = m_pModelCom->GetAnimationClipByName("A_P1011_V17_C00_AtkAngry01_Cut");
	angryAnimCut->SetLoop(false);
	size_t angryCutIdx = ctrl->AddState("AngryCut", angryAnimCut, m_pModelCom->GetAnimationMap()[angryAnimCut->Get_Name()]);

	auto angryAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V17_C00_AtkAngry01_1");
	angryAnim->SetLoop(false);
	size_t angryIdx = ctrl->AddState("Angry", angryAnim, m_pModelCom->GetAnimationMap()[angryAnim->Get_Name()]);


	// 상태 등록
	m_pAnimatorCom->AddTrigger("OpenStart");
	m_pAnimatorCom->AddTrigger("OpenEnd");
	m_pAnimatorCom->AddTrigger("Hurt");
	m_pAnimatorCom->AddTrigger("FreezAttackStart");
	m_pAnimatorCom->AddTrigger("AngryCutStart");
	m_pAnimatorCom->AddBool("Open");

	// 기본으로 끝나고 애니메이션 처리
	AniCon cFinished{ "", CAnimController::EOp::Finished, 0.f };
	// 상태 전환
	AniCon cOpenStart{ "OpenStart", CAnimController::EOp::Trigger, 0.f };

	ctrl->AddTransition(idleIdx, openIdx, cOpenStart, 0.1f); // 딱 열리는 모션
	ctrl->AddTransition(openIdx, openIdx1, cFinished, 0.1f); // 열리 모션 끝나고 Loop

	AniCon cOpen{ "Open", CAnimController::EOp::IsFalse, 0.f };
	ctrl->AddTransition(openIdx1, openIdx2, cOpen, 0.1f); // 열리는 애니메이션
	ctrl->AddTransition(openIdx2, idleIdx, cFinished, 0.1f); // 열리는거

	AniCon cHurt{ "Hurt", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, damgeIdx, cHurt, 0.1f);
	ctrl->AddTransition(damgeIdx, idleIdx, cFinished, 0.1f); // 닫히고 돌아오기


}

void CEnmuBody::Ready_Collider()
{
	if (FAILED(Add_Component(TEXT("Com_Collider"), CSphereCollider::Create(m_pDevice, m_pContext, 50.f), reinterpret_cast<CComponent**>(&m_pColliderCom))))
		return;
	m_pColliderCom->Initialize(nullptr);
	m_pColliderCom->SetColliderType(ColliderType::HURTBOX);
	m_pColliderCom->SetOffset(_float3(0.f, 180.f, 0.f));
	m_pColliderCom->SetListener(this); // 콜라이더 리스너 등록
	m_pColliderCom->SetPriority(5);

}

void CEnmuBody::ActiveCollider()
{
	if (m_pColliderCom)
	{
		m_pColliderCom->SetActive(true);
		m_pColliderCom->SetDrawDebug(true); // 디버그용으로 활성화
	}
}

void CEnmuBody::DeactiveCollider()
{
	if (m_pColliderCom)
	{
		m_pColliderCom->SetActive(false);
		m_pColliderCom->SetDrawDebug(false); // 디버그용으로 비활성화
	}
}

CEnmuBody* CEnmuBody::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CEnmuBody* pInstance = new CEnmuBody(pDevice, pContext);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CEnmuBody");
		Safe_Release(pInstance);
	}
	return pInstance;
}

// 딱히 클론할 필요가 없음
CGameObject* CEnmuBody::Clone(void* pArg)
{
	return nullptr;
}

void CEnmuBody::Free()
{
	CEnmuParts::Free();
}

void CEnmuBody::OnCollisionEnter(CCollider* other)
{
	//if (other->GetType() == ColliderType::HITBOX)
	//{
	//	if (other->GetOwner()->Get_Name() == TEXT("Weapon"))
	//	{
	//		auto pEnmu = dynamic_cast<CEnmuMeat*>(GetParent());
	//		if (pEnmu)
	//		{
	//			pEnmu->Hit(10.f); // 데미지 처리
	//		}
	//	}
	//}
}

void CEnmuBody::OnCollisionStay(CCollider* other, float fTimeDelta)
{
}

void CEnmuBody::OnCollisionExit(CCollider* other)
{
}
