#include "Kyojuro.h"
#include "BodyColliderParts.h"
#include "GameInstance.h"
#include "UIProgressBar.h"
#include "InputBuffer.h"
#include <JsonLoader.h>
#include "Animation.h"
#include "Navigation.h"
#include "StateIdle.h"
#include "StateHurt.h"
#include "Weapon.h"	

using AniCon = CAnimController::Condition;
CKyojuro::CKyojuro(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBaseCharacter(pDevice, pContext)
{

}

CKyojuro::CKyojuro(const CKyojuro& Prototype)
	: CBaseCharacter(Prototype)
{
}
HRESULT CKyojuro::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CKyojuro::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};

	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 40.f;
	Desc.strName = TEXT("Kyojuro");


	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(0.1f, 0.1f, 0.1f));


	CGameObject* pWeapon = m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::GAMEPLAY), TEXT("Weapon"));

	if (pWeapon == nullptr)
		pWeapon = m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::ENMU_BOSS), TEXT("Weapon"));

	//	Set_Weapon("R_Hand_1", dynamic_cast<CWeapon*>(pWeapon));
	Set_Weapon("R_Hand_1_Lct", dynamic_cast<CWeapon*>(pWeapon));

	if (m_pWeapon) // 자식벡터로 넣지는 않음
	{
		m_pWeapon->SetParent(this);
	}

	
	ReadyAnimEvents();
	Ready_Animation();


	Add_Component(TEXT("Com_CapsuleCollider"), CCapsuleCollider::Create(m_pDevice, m_pContext, 3.5f, 77.f), reinterpret_cast<CComponent**>(&m_pColliderCom));

	m_pColliderCom->Initialize(nullptr);
	m_pColliderCom->SetOffset(_float3(0.f, 8.1f, 0.f));
	m_pColliderCom->SetListener(this);
	Add_Component(TEXT("Com_RangeCollider"), CSphereCollider::Create(m_pDevice, m_pContext,15.f), reinterpret_cast<CComponent**>(&m_pRangeColliderCom));

	m_pRangeColliderCom->Initialize(nullptr);
	m_pRangeColliderCom->SetOffset(_float3(0.f, 16.f, 0.f));
	m_pRangeColliderCom->SetListener(this);
	m_pRangeColliderCom->SetColliderType(ColliderType::RANGE);
	m_pRangeColliderCom->SetActive(false); // 초기에는 비활성화

	CBodyColliderParts::BODYCOLLIDERPARTS_DESC desc{};
	desc.fRadius = 5.f;
	desc.vColliderOffsets.push_back(_float3(0.f, 0.f, 0.f));
	AddChild(CBodyColliderParts::Create(m_pDevice, m_pContext));

	m_vecChildren.back()->Initialize(&desc);
	if (auto parts = dynamic_cast<CBodyColliderParts*>(m_vecChildren.back()))
	{
		CBone* pBoneRHand = m_pModelCom->Get_Bone("R_Foot_1");
		if (!pBoneRHand)
		{
			MSG_BOX("CKyojuro::Initialize - Bone not found");
			return E_FAIL;
		}
		parts->Set_BoneSocket(pBoneRHand);
		parts->SetActive(false);
	}

	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_CapsuleCollider"),
	//	TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom))))
	//	return E_FAIL;

	ChangeState(new StateIdle(TEXT("Idle")));
	m_iShaderPass = 2;

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(100.f, 0.f, 50.f, 1.f));
	m_pTransformCom->Rotate_EulerAngles(_float3(0.f, -45.f, 0.f));
	if (m_pNavigationCom)
	{
		m_pNavigationCom->FindIndexCell(m_pTransformCom->Get_State(STATE::POSITION));
	}
	return S_OK;
}

void CKyojuro::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CKyojuro::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	for (auto& child : m_vecChildren)
	{
		child->Update(fTimeDelta);
	}

}

void CKyojuro::Late_Update(_float fTimeDelta)
{


	__super::Late_Update(fTimeDelta);
	for (auto& child : m_vecChildren)
	{
		child->Late_Update(fTimeDelta);
	}
//
//	if (m_pState)
//	{
//		auto currentState = m_pState->GetStateName();
//
//		// 현재 애니메이션 상태를 윈도우 타이틀에 표시
//		SetWindowTextA(g_hWnd, WStringToString(currentState).c_str());
//
////		char buf[MAX_PATH];
////		sprintf_s(buf, "현재 애니메이션: %s", currentState);
////		SetWindowTextA(g_hWnd, buf);
//	}
}

HRESULT CKyojuro::Render()
{
	if (m_pNavigationCom)
		m_pNavigationCom->Render();
	__super::Render();
	return S_OK;
}

void CKyojuro::TakeDamage(_float fDamage)
{
	if (m_eState == CSTATE::GUARD)
	{
		fDamage *= 0.5f; // 가드 중에는 피해량 감소
	}
	if (m_eState == CSTATE::SKILL2)
		return;
	__super::TakeDamage(fDamage);
	auto pLeftBar = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("LeftLifeBar"));
	if (!m_bAirborne && !m_bIsBound)
	{
		ChangeState(new StateHurt());
	}
	if (pLeftBar)
	{
		CUIProgressBar* pLifeBar = static_cast<CUIProgressBar*>(pLeftBar);
		pLifeBar->ApplyDamage(fDamage);
	}
}

void CKyojuro::OnAttackHit(CGameObject* pTarget)
{
	if (pTarget)
	{
		switch (m_eState)
		{
		case CSTATE::ATTACK:
			if (auto pCharacter = dynamic_cast<CBaseCharacter*>(pTarget))
			{
				pCharacter->TakeDamage(3.f);
				auto pState = pCharacter->GetState();
				if (pCharacter->IsAirborne())
				{
					pCharacter->LaunchAirborne(40.f);
					pCharacter->PushBack(this);
				}
			}
			break;
		case CSTATE::ATTACK2:
			if (auto pCharacter = dynamic_cast<CBaseCharacter*>(pTarget))
			{
				pCharacter->TakeDamage(3.5f);
				if (pCharacter->IsAirborne())
				{
					pCharacter->LaunchAirborne(40.f);
					pCharacter->PushBack(this);
				}
			}
			break;
		case CSTATE::ATTACK3:
			if (auto pCharacter = dynamic_cast<CBaseCharacter*>(pTarget))
			{
				pCharacter->TakeDamage(3.f);
				if (pCharacter->IsAirborne())
				{
					pCharacter->LaunchAirborne(40.f);
					pCharacter->PushBack(this);
				}
			}
			break;
		case CSTATE::ATTACK4:
			if (auto pCharacter = dynamic_cast<CBaseCharacter*>(pTarget))
			{
				pCharacter->TakeDamage(2.5f);
				if (pCharacter->IsAirborne())
				{
					pCharacter->LaunchAirborne(40.f);
					pCharacter->PushBack(this);
				}
			}
			break;
		case CSTATE::ATTACK_DOWN:
			if (auto pCharacter = dynamic_cast<CBaseCharacter*>(pTarget))
			{
				pCharacter->TakeDamage(5.f);
			}
			break;
		case CSTATE::ATTACK_UP:
			if (auto pCharacter = dynamic_cast<CBaseCharacter*>(pTarget))
			{
				pCharacter->TakeDamage(6.f);
				m_bCanBlowAttack = true;
			}
			break;
		case CSTATE::SKILL:
			if (auto pCharacter = dynamic_cast<CBaseCharacter*>(pTarget))
			{
				pCharacter->TakeDamage(10.f);
				pCharacter->LaunchAirborne(40.f);
			}
			break;
		case CSTATE::SKILL1:
			if (auto pCharacter = dynamic_cast<CBaseCharacter*>(pTarget))
			{
				// 바운드가 아닐 때 히트 판정을 낼 수 있음
				if (pCharacter->GetState() != CBaseCharacter::CSTATE::BOUND)
				{
					pCharacter->LaunchAirborne(60.f,true);
					pCharacter->TakeDamage(10.f);
				}
			}
			break;
		case CSTATE::SKILL2:
			if (auto pCharacter = dynamic_cast<CBaseCharacter*>(pTarget))
			{
			
				m_bCanRangeAttack = true; // 스킬 사용 후 다음 공격 가능
			}
			break;
		}
	}
}

void CKyojuro::OnCollisionEnter(CCollider* other)
{
	CBaseCharacter::OnCollisionEnter(other);
}



HRESULT CKyojuro::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_Toon"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Kyoujuro"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
			return E_FAIL;
	}
	if (m_pGameInstance->Get_CurrentLevelIndex() == 3)
	{

		
	}

	/* For.Com_Navigation */
	CNavigation::NAVIGATION_DESC		NaviDesc{};
	NaviDesc.iIndex = 4;

	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom), &NaviDesc)))
		return E_FAIL;



	return S_OK;
}

void CKyojuro::ReadyAnimEvents()
{// 애니메이션 이벤트 등록
	m_pAnimatorCom->RegisterEventListener("ActiveHitbox", [&](const string&) {
		ActiveCollider();
		});

	m_pAnimatorCom->RegisterEventListener("DeactiveHitbox", [&](const string&) {
		DeactiveCollider();
		});

	m_pAnimatorCom->RegisterEventListener("GuardSkill", [&](const string&) {
		m_Velocity.y = 50.f;
		m_bAirborne = true;
		});


	m_pAnimatorCom->RegisterEventListener("ActiveRangebox", [&](const string&) {
		if (m_pRangeColliderCom)
		{
			m_pRangeColliderCom->SetActive(true);
			m_pRangeColliderCom->SetDrawDebug(true);
		}
		});
	m_pAnimatorCom->RegisterEventListener("DeactiveRangebox", [&](const string&) {
		if (m_pRangeColliderCom)
		{
			m_pRangeColliderCom->SetActive(false);
			m_pRangeColliderCom->SetDrawDebug(false);
		}
		});

	m_pAnimatorCom->RegisterEventListener("GuardSkillAirborne", [&](const string& eventName) {

		if (auto pChar = dynamic_cast<CBaseCharacter*>(m_pTarget))
		{
			if (m_bCanRangeAttack)
			{
				pChar->LaunchAirborneFall(60.f);
				pChar->LaunchAirborneFall(20.f);
				pChar->TakeDamage(20.f);
				m_bCanRangeAttack = false;
			}
		}

		});

}

void CKyojuro::Ready_Animation()
{
	m_pAnimatorCom->PlayClip(m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseNut01_1"));

	auto animations = m_pModelCom->GetAnimations();
	CJsonLoader jsonLoader;
	jsonLoader.LoadAnimEvent("../Asset/Json/Kyojuro_events.json", animations);
	jsonLoader.Free();

	auto ctrl = m_pAnimatorCom->GetAnimController();
	// 2) 상태(State) 등록
	   // Idle
	auto idleAnim = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseNut01_1");
	idleAnim->SetLoop(true);
	size_t idleIdx = ctrl->AddState("Idle", idleAnim, m_pModelCom->GetAnimationMap()[idleAnim->Get_Name()]);
	idleAnim->AddEvent({ 0.f, "DeactiveHitbox" }); // 0 프레임에 Hitbox 활성화

	// Run
	auto runAnim = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseRun01_1");
	runAnim->SetLoop(true);
	size_t runIdx = ctrl->AddState("Run", runAnim, 1);

	// RunEnd (달리다 멈추는)  논루프
	auto runEndAnim = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseRun01_2");
	runEndAnim->SetLoop(false);
	size_t runEndIdx = ctrl->AddState("RunEnd", runEndAnim, 1);

	// Jump0~3
	vector<CAnimation*> jumpClips;
	for (int i = 0; i < 4; ++i)
	{
		auto name = "A_P0012_V00_C00_BaseJump01_" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name.c_str());
		a->SetLoop(false);
		jumpClips.push_back(a);
	}
	size_t jump0Idx = ctrl->AddState("Jump0", jumpClips[0], 2);
	size_t jump1Idx = ctrl->AddState("Jump1", jumpClips[1], 2);
	size_t jump3Idx = ctrl->AddState("Jump3", jumpClips[3], 2);
	jumpClips[1]->SetTickPerSecond(20.f); // 1번 점프 속도
	jumpClips[2]->SetTickPerSecond(45.f); 
	jumpClips[3]->SetTickPerSecond(45.f); 

	vector<CAnimation*> comboAttackClips;
	for (int i = 1; i <= 4; ++i)
	{
		auto name = "A_P0012_V00_C00_AtkCmbW0" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name.c_str());
		a->SetLoop(false);
		comboAttackClips.push_back(a);
	}
	auto attackDown = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_AtkCmbW03D01");
	attackDown->SetLoop(false);
	comboAttackClips.push_back(attackDown);

	auto attackUp = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_AtkCmbW03U01");
	attackUp->SetLoop(false);
	comboAttackClips.push_back(attackUp);


	comboAttackClips[2]->SetTickPerSecond(25.f); // 3타 공격 속도
	comboAttackClips[4]->SetTickPerSecond(20.f); // 아래 공격 속도
	comboAttackClips[5]->SetTickPerSecond(20.f); // 위 공격 속도
	size_t attack0Idx = ctrl->AddState("attack0", comboAttackClips[0], 3);
	size_t attack1Idx = ctrl->AddState("attack1", comboAttackClips[1], 3);
	size_t attack2Idx = ctrl->AddState("attack2", comboAttackClips[2], 3);
	size_t attack3Idx = ctrl->AddState("attack3", comboAttackClips[3], 3);
	size_t attack4Idx = ctrl->AddState("attackDown", comboAttackClips[4], 3);
	size_t attack5Idx = ctrl->AddState("attackUp", comboAttackClips[5], 3);

	comboAttackClips[5]->AddEvent({36.f , "DeactiveHitbox"}); // 36 프레임

	// Idle → Attack0 : 버튼 누르면 즉시 진입
	AniCon c0{ "Attack", CAnimController::EOp::Trigger, 0.f };
	c0.minTime = 0.1f; c0.maxTime = 1.f;
	ctrl->AddTransition(idleIdx, attack0Idx, c0, 0.1f);

	// Attack0 → Attack1 : 20~60% 구간에만
	AniCon c1{ "Attack", CAnimController::EOp::Trigger, 0.f };
	c1.minTime = 0.2f; c1.maxTime = 0.6f;
	ctrl->AddTransition(attack0Idx, attack1Idx, c1, 0.1f);

	// Attack1 → Attack2 : 20~60% 구간
	AniCon c2{ "Attack", CAnimController::EOp::Trigger, 0.f };
	c2.minTime = 0.2f; c2.maxTime = 0.6f;
	ctrl->AddTransition(attack1Idx, attack2Idx, c2, 0.1f);

	// Attack2 → Attack3 : 20~60% 구간
	AniCon c3{ "Attack", CAnimController::EOp::Trigger, 0.f };
	c3.minTime = 0.4f; c3.maxTime = 1.f;
	ctrl->AddTransition(attack2Idx, attack3Idx, c3, 0.1f);


	//A_P0000_V00_C00_Dmg01_F


	auto animHurtFront = m_pModelCom->GetAnimationClipByName("A_P0000_V00_C00_Dmg01_F");
	animHurtFront->SetLoop(false);
	size_t hurtFIdx = ctrl->AddState("Hurt_F", animHurtFront, 12);

	// A_P0012_V00_C00_BaseGuard01_0


	vector<CAnimation*> guardClips;
	for (int i = 0; i < 3; ++i)
	{
		auto name = "A_P0012_V00_C00_BaseGuard01_" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name.c_str());

		if (i == 1)
			a->SetLoop(true);
		else
			a->SetLoop(false);
		guardClips.push_back(a);
	}

	guardClips[0]->SetTickPerSecond(40.f); // 0번은 빠르게
	size_t guard0Idx = ctrl->AddState("guard0", guardClips[0], 4);
	size_t guard1Idx = ctrl->AddState("guard1", guardClips[1], 4);
	size_t guard2Idx = ctrl->AddState("guard2", guardClips[2], 4);


	vector<CAnimation*> guardSkillClips;
	for (int i = 0; i < 3; ++i)
	{
		auto name = "A_P0012_V00_C00_AtkUniqueAct01_" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name.c_str());
		a->SetLoop(false);
		guardSkillClips.push_back(a);
	}
	guardSkillClips[1]->SetTickPerSecond(40.f); // 1번 스킬 속도 조정
	// A_P0012_V00_C00_AtkUniqueAct01_0 상승염천
	size_t guardSkill0Idx = ctrl->AddState("guardSkill0", guardSkillClips[0], 5);
	size_t guardSkill1Idx = ctrl->AddState("guardSkill1", guardSkillClips[1], 5);
	size_t guardSkill2Idx = ctrl->AddState("guardSkill2", guardSkillClips[2], 5);

	auto animStep = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseStepB01");
	animStep->SetLoop(false);
	size_t stepBackIdx = ctrl->AddState("stepBack", animStep, 7);

	auto animStep2 = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseStepF01");
	animStep2->SetLoop(false);
	size_t stepFrontIdx = ctrl->AddState("stepFront", animStep2, 7);

	auto animStep3 = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseStepL01");
	animStep3->SetLoop(false);
	size_t stepLeftIdx = ctrl->AddState("stepLeft0", animStep3, 7);

	auto animStep4 = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseStepL02");
	animStep4->SetLoop(false);
	size_t stepLeftIdx2 = ctrl->AddState("stepLeft1", animStep4, 7);

	auto animStep5 = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseStepR01");
	animStep5->SetLoop(false);
	size_t stepRightIdx = ctrl->AddState("stepRight", animStep5, 7);
	auto animStep6 = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseStepR02");
	animStep6->SetLoop(false);
	size_t stepRightIdx2 = ctrl->AddState("stepRight2", animStep6, 7);

	auto animStep7 = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseStepAR01");
	animStep7->SetLoop(false);
	size_t stepRightJumpIdx = ctrl->AddState("stepRightJump", animStep7, 7);

	auto animStep8 = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseStepAL01");
	animStep8->SetLoop(false);
	size_t stepLeftJumpIdx = ctrl->AddState("stepLeftJump", animStep8, 7);

	auto animStep9 = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseStepAB01");
	animStep9->SetLoop(false);
	size_t stepBackJumpIdx = ctrl->AddState("stepBackJump", animStep9, 7);

	auto animStep10 = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseStepAF01");
	animStep10->SetLoop(false);
	size_t stepFrontJumpIdx = ctrl->AddState("stepFrontJump", animStep10, 7);



	// A_P0012_V00_C00_AtkSkl02 염호
	auto skillDefault = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_AtkSkl02");
	skillDefault->SetLoop(false);
	skillDefault->SetTickPerSecond(35.f); // 스킬 속도 조정
	size_t skillDefaultIdx = ctrl->AddState("skill0", skillDefault, 8);

	// A_P0012_V00_C00_AtkSkl03_0 기염만상
	vector<CAnimation*> skill1Clips;

	for (auto i = 0; i < 2; i++)
	{
		auto name = "A_P0012_V00_C00_AtkSkl03_" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name);
		a->SetLoop(false);
		skill1Clips.push_back(a);
	}
	size_t skill1Idx = ctrl->AddState("skill1", skill1Clips[0], 10);
	size_t skill1EndIdx = ctrl->AddState("skill1End", skill1Clips[1], 10);
	skill1Clips[0]->SetTickPerSecond(30.f); // 스킬 속도 조정
	skill1Clips[1]->SetTickPerSecond(60.f); 



	// 3) 파라미터(Parameter) 등록
	m_pAnimatorCom->AddBool("Move");
	m_pAnimatorCom->AddBool("Jump");
	m_pAnimatorCom->AddBool("Guard");
	m_pAnimatorCom->AddBool("Attacking");
	m_pAnimatorCom->AddTrigger("JumpAttack");
	m_pAnimatorCom->AddTrigger("Attack");
	m_pAnimatorCom->AddTrigger("AttackDown");
	m_pAnimatorCom->AddTrigger("AttackUp");
	m_pAnimatorCom->AddTrigger("Skill0");
	m_pAnimatorCom->AddTrigger("Skill1");
	m_pAnimatorCom->AddTrigger("Skill2");
	m_pAnimatorCom->AddTrigger("DashAttack");
	m_pAnimatorCom->AddTrigger("StepBack");
	m_pAnimatorCom->AddTrigger("StepFront");
	m_pAnimatorCom->AddTrigger("StepLeft");
	m_pAnimatorCom->AddTrigger("StepLeft2");
	m_pAnimatorCom->AddTrigger("StepRight");
	m_pAnimatorCom->AddTrigger("StepRight2");
	m_pAnimatorCom->AddTrigger("StepRightJump");
	m_pAnimatorCom->AddTrigger("StepLeftJump");
	m_pAnimatorCom->AddTrigger("StepBackJump");
	m_pAnimatorCom->AddTrigger("StepFrontJump");
	m_pAnimatorCom->AddTrigger("Hurt");
	m_pAnimatorCom->AddBool("Stepping"); // 스텝 중인지 여부
	m_pAnimatorCom->AddBool("Hurted");


	// 추적 대시 A_P0012_V00_C00_AtkSkl01
	auto dashAttackAnim = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_AtkSkl01");
	dashAttackAnim->SetLoop(false);
	size_t dashAttackIdx = ctrl->AddState("dashAttack", dashAttackAnim, 6);


	// Idle → Run 
	CAnimController::Condition cSpeedUp{ "Move", CAnimController::EOp::IsTrue, 0.1f };
	ctrl->AddTransition(idleIdx, runIdx, cSpeedUp, 0.01f);
	ctrl->AddTransition(runEndIdx, runIdx, cSpeedUp, 0.1f);
	// Run → RunEnd 
	CAnimController::Condition cSpeedDown{ "Move", CAnimController::EOp::IsFalse, 0.1f };

	ctrl->AddTransition(runIdx, runEndIdx, cSpeedDown, 0.1f);

	// RunEnd → Idle : Finished (애니메이션 종료 시)
	CAnimController::Condition cFinished{ "", CAnimController::EOp::Finished, 0.f };
	ctrl->AddTransition(runEndIdx, idleIdx, cFinished);

	CAnimController::Condition cFin{ "", CAnimController::EOp::Finished, 0.f };

	// 3타에서 아래로
	CAnimController::Condition cAttackDown{ "AttackDown", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(attack2Idx, attack4Idx, cAttackDown, 0.25f);
	// 3타에서 위로
	CAnimController::Condition cAttackUp{ "AttackUp", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(attack2Idx, attack5Idx, cAttackUp, 0.25f);
	// Attack0 → Idle

	ctrl->AddTransition(attack0Idx, runIdx, cSpeedUp);
	ctrl->AddTransition(attack0Idx, idleIdx, cFin);

	// Attack1 → Idle
	ctrl->AddTransition(attack1Idx, runIdx, cSpeedUp);
	ctrl->AddTransition(attack1Idx, idleIdx, cFin);

	// Attack2 → Idle
	ctrl->AddTransition(attack2Idx, runIdx, cSpeedUp);
	ctrl->AddTransition(attack2Idx, idleIdx, cFin);

	// Attack3 → Idle
	ctrl->AddTransition(attack3Idx, runIdx, cSpeedUp);
	ctrl->AddTransition(attack3Idx, idleIdx, cFin); // 기본 마지막 공격

	// AttackDown → Idle
	ctrl->AddTransition(attack4Idx, runIdx, cSpeedUp);
	ctrl->AddTransition(attack4Idx, idleIdx, cFin);

	// AttackUp -> Idle
	ctrl->AddTransition(attack5Idx, runIdx, cSpeedUp);
	ctrl->AddTransition(attack5Idx, idleIdx, cFin);


	ctrl->AddTransition(runIdx, attack0Idx, c1, 0.1f);
	ctrl->AddTransition(runEndIdx, attack0Idx, c1, 0.1f);



	ctrl->AddTransition(attack0Idx, runIdx, cSpeedUp);
	ctrl->AddTransition(attack1Idx, runIdx, cSpeedUp);
	ctrl->AddTransition(attack2Idx, runIdx, cSpeedUp);
	ctrl->AddTransition(attack3Idx, runIdx, cSpeedUp); // 기본 마지막 공격
	ctrl->AddTransition(attack5Idx, runIdx, cSpeedUp);



	// 점프 처리

	//A_P0012_V00_C00_AtkCmbAW01 점프 중 공격

	auto jumpAttack = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_AtkCmbAW01");
	jumpAttack->SetLoop(false);
	jumpAttack->SetTickPerSecond(35.f); // 점프 공격 속도 조정
	size_t jumpAttackIdx = ctrl->AddState("jumpAttack", jumpAttack, 9);
	CAnimController::Condition cJumpAttack{ "JumpAttack", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(jump0Idx, jumpAttackIdx, cJumpAttack, 0.1f);
	ctrl->AddTransition(jump1Idx, jumpAttackIdx, cJumpAttack, 0.1f);
//	ctrl->AddTransition(jump2Idx, jumpAttackIdx, cJumpAttack, 0.1f);
	ctrl->AddTransition(jump3Idx, jumpAttackIdx, cJumpAttack, 0.1f);



	// Idle/Run → Jump0 : Jump Trigger
	CAnimController::Condition cJump{ "Jump", CAnimController::EOp::IsTrue, 0.f };
	ctrl->AddTransition(runEndIdx, jump0Idx, cJump);
	ctrl->AddTransition(runIdx, jump0Idx, cJump);
	ctrl->AddTransition(idleIdx, jump0Idx, cJump);

	// Jump0 → Jump1, Jump1 → Jump2, Jump2 → Jump3 : Finished
	ctrl->AddTransition(jump0Idx, jump1Idx, cFinished, 0.05f);
	ctrl->AddTransition(jump1Idx, jump3Idx, cFinished, 0.05f);
	//ctrl->AddTransition(jump2Idx, jump3Idx, cFinished, 0.05f);
	// Jump0 -> JumpAttack ->Jump2->Jump3
	ctrl->AddTransition(jumpAttackIdx, jump3Idx, cFinished, 0.05f);

	// Jump0 → Run
	ctrl->AddTransition(jump3Idx, runIdx, cSpeedUp, 0.1f);
	// Jump3 → Idle : Finished
	ctrl->AddTransition(jump3Idx, idleIdx, cFinished, 0.1f);



	// 가드 진입 (Idle / Run / RunEnd → Guard0)
	{
		CAnimController::Condition cG{ "Guard", CAnimController::EOp::IsTrue, 0.f };
		// 눌렀을 때 즉시
		ctrl->AddTransition(runEndIdx, guard0Idx, cG, 0.1f);
		ctrl->AddTransition(runIdx, guard0Idx, cG, 0.1f);
		ctrl->AddTransition(idleIdx, guard0Idx, cG, 0.1f);
	}

	// Guard0 → Guard1 → Guard2: 논루프 스테이트가 끝나면 자동 진행
	{

		CAnimController::Condition cFin{ "", CAnimController::EOp::Finished, 0.f };
		ctrl->AddTransition(guard0Idx, guard1Idx, cFin, 0.1f);
	}
	// 눌러 있는 동안만 유지, 떼면 즉시 풀림 (Guard → Idle)
	{
		CAnimController::Condition cGU{ "Guard", CAnimController::EOp::IsFalse, 0.f };

		ctrl->AddTransition(guard0Idx, guard2Idx, cGU, 0.05f);
		ctrl->AddTransition(guard1Idx, guard2Idx, cGU, 0.05f);
		ctrl->AddTransition(guard0Idx, idleIdx, cGU, 0.05f);
		ctrl->AddTransition(guard1Idx, idleIdx, cGU, 0.05f);
		ctrl->AddTransition(guard2Idx, idleIdx, cGU, 0.05f);
	}

	// 염호 스킬 상태 설정
	CAnimController::Condition cFlameTiger{ "Skill0", CAnimController::EOp::Trigger, 0.f };

	ctrl->AddTransition(attack0Idx, skillDefaultIdx, cFlameTiger);
	ctrl->AddTransition(attack1Idx, skillDefaultIdx, cFlameTiger);
	ctrl->AddTransition(attack2Idx, skillDefaultIdx, cFlameTiger);
	ctrl->AddTransition(attack3Idx, skillDefaultIdx, cFlameTiger);
	ctrl->AddTransition(attack4Idx, skillDefaultIdx, cFlameTiger);
	ctrl->AddTransition(attack5Idx, skillDefaultIdx, cFlameTiger);
	ctrl->AddTransition(skillDefaultIdx, runIdx, cSpeedUp);
	ctrl->AddTransition(idleIdx, skillDefaultIdx, cFlameTiger, 0.2f);
	ctrl->AddTransition(skillDefaultIdx, idleIdx, cFin);

	// 기염만상
	CAnimController::Condition cBreath{ "Skill1", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(attack0Idx, skill1Idx, cBreath);
	ctrl->AddTransition(attack1Idx, skill1Idx, cBreath);
	ctrl->AddTransition(attack2Idx, skill1Idx, cBreath);
	ctrl->AddTransition(attack3Idx, skill1Idx, cBreath);
	ctrl->AddTransition(attack4Idx, skill1Idx, cBreath);
	ctrl->AddTransition(attack5Idx, skill1Idx, cBreath);
	ctrl->AddTransition(runEndIdx, skill1Idx, cBreath);
	ctrl->AddTransition(skill1Idx, skill1EndIdx, cFin, 0.1f);
	ctrl->AddTransition(skill1EndIdx, runIdx, cSpeedUp);
	ctrl->AddTransition(skill1EndIdx, idleIdx, cFin);

	// 상승염천
	CAnimController::Condition cRisingSun{ "Skill2", CAnimController::EOp::Trigger, 0.f };
	//ctrl->AddTransition(attack0Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack1Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack2Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack3Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack4Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack5Idx, skillDefaultIdx, cRisingSun);
	ctrl->AddTransition(idleIdx, guardSkill0Idx, cRisingSun, 0.1f);
	ctrl->AddTransition(guard0Idx, guardSkill0Idx, cRisingSun, 0.1f);
	ctrl->AddTransition(guard1Idx, guardSkill0Idx, cRisingSun, 0.1f);
	ctrl->AddTransition(guard2Idx, guardSkill0Idx, cRisingSun, 0.1f);
	ctrl->AddTransition(guardSkill0Idx, guardSkill1Idx, cFin, 0.1f);
	ctrl->AddTransition(guardSkill1Idx, guardSkill2Idx, cFin, 0.1f);
	ctrl->AddTransition(guardSkill2Idx, runIdx, cSpeedUp);
	ctrl->AddTransition(guardSkill2Idx, idleIdx, cFin);

	// 따라가는 대시 공격 설정

	CAnimController::Condition DashAttack{ "DashAttack", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, dashAttackIdx, DashAttack, 0.1f);
	ctrl->AddTransition(dashAttackIdx, idleIdx, cFin);
	ctrl->AddTransition(dashAttackIdx, runIdx, cFin, 0.1f);


	// 스탭 처리
	CAnimController::Condition StepBack{ "StepBack", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, stepBackIdx, StepBack, 0.1f);
	ctrl->AddTransition(runIdx, stepBackIdx, StepBack, 0.1f);
	ctrl->AddTransition(stepBackIdx, runIdx, cFin);
	ctrl->AddTransition(stepBackIdx, idleIdx, cSpeedDown);

	CAnimController::Condition StepFront{ "StepFront", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, stepFrontIdx, StepFront, 0.1f);
	ctrl->AddTransition(runIdx, stepFrontIdx, StepFront, 0.1f);
	ctrl->AddTransition(stepFrontIdx, runIdx, cFin);
	ctrl->AddTransition(stepFrontIdx, idleIdx, cSpeedDown);

	CAnimController::Condition StepLeft{ "StepLeft", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, stepLeftIdx, StepLeft, 0.1f);
	ctrl->AddTransition(runIdx, stepLeftIdx, StepLeft, 0.1f);
	ctrl->AddTransition(stepLeftIdx, runIdx, cFin);
	ctrl->AddTransition(stepLeftIdx, idleIdx, cSpeedDown);

	CAnimController::Condition StepRight{ "StepRight", CAnimController::EOp::Trigger, 0.f };

	ctrl->AddTransition(idleIdx, stepRightIdx, StepRight, 0.1f);
	ctrl->AddTransition(runIdx, stepRightIdx, StepRight, 0.1f);
	ctrl->AddTransition(stepRightIdx, runIdx, cFin);
	ctrl->AddTransition(stepRightIdx, idleIdx, cSpeedDown);

	CAnimController::Condition StepRight2{ "StepRight2", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(stepRightIdx, stepRightIdx2, StepRight2);
	ctrl->AddTransition(stepRightIdx2, stepRightIdx, StepLeft);
	ctrl->AddTransition(stepRightIdx2, idleIdx, cFin);

	CAnimController::Condition StepLeft2{ "StepLeft2", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(stepLeftIdx, stepLeftIdx2, StepLeft2);
	ctrl->AddTransition(stepLeftIdx2, stepLeftIdx, StepLeft);
	ctrl->AddTransition(stepLeftIdx2, idleIdx, cFin);

	CAnimController::Condition StepRightJump{ "StepRightJump", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(jump0Idx, stepRightJumpIdx, StepRightJump, 0.1f);
	ctrl->AddTransition(jump1Idx, stepRightJumpIdx, StepRightJump, 0.1f);
	//ctrl->AddTransition(jump2Idx, stepRightJumpIdx, StepRightJump, 0.1f);
	ctrl->AddTransition(jump3Idx, stepRightJumpIdx, StepRightJump, 0.1f);
//	ctrl->AddTransition(stepRightJumpIdx, jump2Idx, cFinished, 0.05f);
	ctrl->AddTransition(stepRightJumpIdx, jump3Idx, cFinished, 0.05f);


	CAnimController::Condition StepLeftJump{ "StepLeftJump", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(jump0Idx, stepLeftJumpIdx, StepLeftJump, 0.1f);
	ctrl->AddTransition(jump1Idx, stepLeftJumpIdx, StepLeftJump, 0.1f);
//	ctrl->AddTransition(jump2Idx, stepLeftJumpIdx, StepLeftJump, 0.1f);
	ctrl->AddTransition(jump3Idx, stepLeftJumpIdx, StepLeftJump, 0.1f);
	//ctrl->AddTransition(stepLeftJumpIdx, jump2Idx, cFinished, 0.05f);
	ctrl->AddTransition(stepLeftJumpIdx, jump3Idx, cFinished, 0.05f);

	CAnimController::Condition StepBackJump{ "StepBackJump", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(jump0Idx, stepBackJumpIdx, StepBackJump, 0.1f);
	ctrl->AddTransition(jump1Idx, stepBackJumpIdx, StepBackJump, 0.1f);
//	ctrl->AddTransition(jump2Idx, stepBackJumpIdx, StepBackJump, 0.1f);
	ctrl->AddTransition(jump3Idx, stepBackJumpIdx, StepBackJump, 0.1f);
//	ctrl->AddTransition(stepBackJumpIdx, jump2Idx, cFinished, 0.05f);
	ctrl->AddTransition(stepBackJumpIdx, jump3Idx, cFinished, 0.05f);

	CAnimController::Condition StepFrontJump{ "StepFrontJump", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(jump0Idx, stepFrontJumpIdx, StepFrontJump, 0.1f);
	ctrl->AddTransition(jump1Idx, stepFrontJumpIdx, StepFrontJump, 0.1f);
//	ctrl->AddTransition(jump2Idx, stepFrontJumpIdx, StepFrontJump, 0.1f);
	ctrl->AddTransition(jump3Idx, stepFrontJumpIdx, StepFrontJump, 0.1f);
//   ctrl->AddTransition(stepFrontJumpIdx, jump2Idx, cFinished, 0.05f);
	ctrl->AddTransition(stepFrontJumpIdx, jump3Idx, cFinished, 0.05f);


	// 피격 처리
	CAnimController::Condition cHurt{ "Hurt", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(runIdx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(runEndIdx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(idleIdx, hurtFIdx, cHurt, 0.1f);

	ctrl->AddTransition(attack0Idx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(attack1Idx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(attack2Idx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(attack3Idx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(attack4Idx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(attack5Idx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(skill1Idx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(skill1EndIdx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(skillDefaultIdx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(guardSkill2Idx, hurtFIdx, cHurt, 0.1f);

	ctrl->AddTransition(hurtFIdx, idleIdx, cFin);
	ctrl->AddTransition(hurtFIdx, runIdx, cSpeedUp, 0.1f);

}

void CKyojuro::ActiveCollider()
{
	if (m_pWeapon)
	{
		auto collider =static_cast<CSphereCollider*>(m_pWeapon->Get_Component(TEXT("Com_Collider")));
		auto collider1 =static_cast<CSphereCollider*>(m_pWeapon->Get_Component(TEXT("Com_Collider1")));
		auto collider2 =static_cast<CSphereCollider*>(m_pWeapon->Get_Component(TEXT("Com_Collider2")));

		if (collider)
		{
			collider->SetActive(true);
			collider->SetDrawDebug(true);
		}
		if (collider1)
		{
			collider1->SetActive(true);
			collider1->SetDrawDebug(true);
		}
		if (collider2)
		{
			collider2->SetActive(true);
			collider2->SetDrawDebug(true);
		}
	}
	for (auto& pBodyColl : m_vecChildren)
	{
		if (dynamic_cast<CBodyColliderParts*>(pBodyColl))
		{
			pBodyColl->SetActive(true);
		}
	}
}

void CKyojuro::DeactiveCollider()
{
	if (m_pWeapon)
	{
		auto collider = static_cast<CSphereCollider*>(m_pWeapon->Get_Component(TEXT("Com_Collider")));
		auto collider1 = static_cast<CSphereCollider*>(m_pWeapon->Get_Component(TEXT("Com_Collider1")));
		auto collider2 = static_cast<CSphereCollider*>(m_pWeapon->Get_Component(TEXT("Com_Collider2")));


		if (collider)
		{
			collider->SetActive(false);
			collider->SetDrawDebug(false);
		}
		if (collider1)
		{
			collider1->SetActive(false);
			collider1->SetDrawDebug(false);
		}
		if (collider2)
		{
			collider2->SetActive(false);
			collider2->SetDrawDebug(false);
		}
		m_pWeapon->ClearDamagedTargets();
	}
	for (auto& pBodyColl : m_vecChildren)
	{
		if (auto bodyCol = dynamic_cast<CBodyColliderParts*>(pBodyColl))
		{
			bodyCol->SetActive(false);
		}
	}
}




CKyojuro* CKyojuro::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKyojuro* pInstance = new CKyojuro(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CKyojuro");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CKyojuro::Clone(void* pArg)
{
	CKyojuro* pInstance = new CKyojuro(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CKyojuro");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CKyojuro::Free()
{
	__super::Free();
}
