#include "Akaza.h"
#include "GameInstance.h"
#include "Animation.h"
#include "StateIdle.h"
#include "InputBuffer.h"
#include "StateMove.h"	
#include "StateHurt.h"
#include "StateAttack1.h"	
#include "StateJump.h"
#include "StateStep.h"
#include "StateGuard.h"
#include "StateSkill0.h"
#include "StateSkill1.h"
#include "StateSkill2.h"
#include "BodyColliderParts.h"
#include "UIProgressBar.h"
#include <JsonLoader.h>
#include "Navigation.h"


using AniCon = CAnimController::Condition;
CAkaza::CAkaza(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBaseCharacter(pDevice, pContext)
{
	random_device rd;
	m_RandGen.seed(rd());
}

CAkaza::CAkaza(const CAkaza& Prototype)
	: CBaseCharacter(Prototype)
{
}
HRESULT CAkaza::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CAkaza::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};

	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 40.f;
	Desc.strName = TEXT("Akaza");

	if (FAILED(Ready_Components()))
		return E_FAIL;


	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(0.1f, 0.1f, 0.1f));


	// 애니메이션 이벤트 등록
	m_pAnimatorCom->RegisterEventListener("ActiveHitbox", [&](const string&) {
		ActiveCollider();
		});

	m_pAnimatorCom->RegisterEventListener("DeactiveHitbox", [&](const string&) {
		DeactiveCollider();
		});



	Ready_Animation();


	Add_Component(TEXT("Com_CapsuleCollider"), CCapsuleCollider::Create(m_pDevice, m_pContext, 3.5f, 77.f), reinterpret_cast<CComponent**>(&m_pColliderCom));

	m_pColliderCom->Initialize(nullptr);
	m_pColliderCom->SetOffset(_float3(0.f, 8.1f, 0.f));

	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_CapsuleCollider"),
	//	TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom))))
	//	return E_FAIL;
	ChangeState(new StateIdle(TEXT("Idle")));
	m_eComState = COM_STATE::IDLE;
	m_pColliderCom->SetListener(this);

	CBodyColliderParts::BODYCOLLIDERPARTS_DESC desc{};
	desc.vColliderOffsets.push_back(_float3(0.f, 0.f, 0.f));
	for (int i = 0; i < 4; i++)
	{
		AddChild(CBodyColliderParts::Create(m_pDevice, m_pContext));
	    m_vecChildren.back()->Initialize(&desc);
	}

	for (int i = 0; i < 4; i++)
	{
		if (auto parts = dynamic_cast<CBodyColliderParts*>(m_vecChildren[i]))
		{
			CBone* pBoneRHand = nullptr;

			if (i == 0)
			{
				pBoneRHand = m_pModelCom->Get_Bone("L_Hand_1");
			}
			else if (i == 1)
			{
				pBoneRHand = m_pModelCom->Get_Bone("R_Hand_1");
			}
			else if (i == 2)
			{
				pBoneRHand = m_pModelCom->Get_Bone("R_Foot_1");
			}
			else if (i == 3)
			{
				pBoneRHand = m_pModelCom->Get_Bone("L_Foot_1");
			}
			if (!pBoneRHand)
			{
				MSG_BOX("CAkaza::Initialize - Bone not found");
				return E_FAIL;
			}
			parts->Set_BoneSocket(pBoneRHand);
		}
	}

	m_iShaderPass = 3;

	if (m_pNavigationCom)
	{
		m_pNavigationCom->FindIndexCell(m_pTransformCom->Get_State(STATE::POSITION));
	}

	m_fMaxHP = 250.f;
	m_fCurrentHP = m_fMaxHP;

	return S_OK;
}

void CAkaza::Priority_Update(_float fTimeDelta)
{
	CBaseCharacter::Priority_Update(fTimeDelta);

	static _uint iAnim = 0;
	if (m_pGameInstance->IsKeyPressed('N'))
	{
		m_pAnimatorCom->Set_Animation(iAnim, 0.15f);
		iAnim++;
	}

	if (m_pGameInstance->IsKeyPressed('M'))
	{
		m_pAnimatorCom->Set_Animation(iAnim, 0.15f);
		iAnim = max(0, iAnim - 1);
	}

	//if (m_fCurrentHP <= 0.f)
	//{
	//	if (m_pState)
	//	{
	//		if (m_pState->GetStateName() != "Die")
	//		{
	//			ChangeState(new StateDie());
	//		}
	//	}
	//	else
	//	{
	//		ChangeState(new StateDie());
	//	}
	//}

}

void CAkaza::Update(_float fTimeDelta)
{
	//__super::Update(fTimeDelta);

	//UpdateState(fTimeDelta);
	//m_pAnimatorCom->GetAnimController()->Update(fTimeDelta);
	//m_pModelCom->Play_Animation(fTimeDelta);

	for (auto& child : m_vecChildren)
	{
		child->Update(fTimeDelta);
	}

}

void CAkaza::Late_Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	__super::Late_Update(fTimeDelta);


	for (auto& child : m_vecChildren)
	{
		child->Late_Update(fTimeDelta);
	}

	if (m_pState)
	{
		auto currentState = m_pState->GetStateName();

		auto animCtrl = m_pAnimatorCom->GetAnimController();
		//if (animCtrl)
		//{
		//	auto currentAnim = m_pAnimatorCom->GetCurrentAnimName();
		//	char buf[MAX_PATH];
		//sprintf_s(buf, "현재 애니메이션: %s", currentAnim);
		//SetWindowTextA(g_hWnd, buf);
		//}

		// 현재 애니메이션 상태를 윈도우 타이틀에 표시
		SetWindowTextA(g_hWnd, WStringToString(currentState).c_str());

		//		char buf[MAX_PATH];
		//		sprintf_s(buf, "현재 애니메이션: %s", currentState);
		//		SetWindowTextA(g_hWnd, buf);
	}
	//PredictPlayerState();
}

HRESULT CAkaza::Render()
{
	__super::Render();
	return S_OK;
}

void CAkaza::TakeDamage(_float fDamage)
{
	__super::TakeDamage(fDamage);
	if(!m_bAirborne&&!m_bIsBound)
	{
		ChangeState(new StateHurt());
	}
	auto pBar = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("RightLifeBar"));
	if (pBar)
	{
		CUIProgressBar* pRightBar = static_cast<CUIProgressBar*>(pBar);
		pRightBar->ApplyDamage(fDamage / m_fMaxHP * 100.f);
	}
}

void CAkaza::OnAttackHit(CGameObject* pTarget)
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
				pCharacter->TakeDamage(5.f);
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
				pCharacter->TakeDamage(8.f);
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
				pCharacter->TakeDamage(5.f);
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
				pCharacter->TakeDamage(30.f);
			}
			break;
		case CSTATE::ATTACK_UP:
			if (auto pCharacter = dynamic_cast<CBaseCharacter*>(pTarget))
			{
				pCharacter->TakeDamage(30.f);
			}
			break;
		case CSTATE::SKILL:
			if (auto pCharacter = dynamic_cast<CBaseCharacter*>(pTarget))
			{
			}
			break;
		case CSTATE::SKILL1:
			if (auto pCharacter = dynamic_cast<CBaseCharacter*>(pTarget))
			{
				// 바운드가 아닐 때 히트 판정을 낼 수 있음
				if (pCharacter->GetState() != CBaseCharacter::CSTATE::BOUND)
				{
					pCharacter->TakeDamage(3.f);
				}
			}
			break;
		case CSTATE::SKILL2:
			if (auto pCharacter = dynamic_cast<CBaseCharacter*>(pTarget))
			{
				pCharacter->TakeDamage(50.f);
			}
			break;
		}
	}
}

void CAkaza::OnCollisionEnter(CCollider* other)
{
	CBaseCharacter::OnCollisionEnter(other);

	if (other->GetType() == ColliderType::RANGE)
	{
		if (auto pAttacker = dynamic_cast<CBaseCharacter*>(other->GetOwner()))
		{
			pAttacker->OnAttackHit(this);
		}
	}
}


HRESULT CAkaza::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_Toon"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Akaza"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	///* For.Com_AnimController*/
	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Animator"),
	//	TEXT("Com_Animator"), reinterpret_cast<CComponent**>(&m_pAnimatorCom), m_pModelCom)))
	//	return E_FAIL;


	//if (m_pGameInstance->Get_CurrentLevelIndex() == 3)
	//{



	//}
	/* For.Com_Navigation */
	CNavigation::NAVIGATION_DESC		NaviDesc{};
	NaviDesc.iIndex = 1;

	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom), &NaviDesc)))
		return E_FAIL;
	return S_OK;
}

void CAkaza::Ready_Animation()
{

	m_pAnimatorCom->PlayClip(m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseNut01_1"));
	auto animations = m_pModelCom->GetAnimations();
	CJsonLoader jsonLoader;
	jsonLoader.LoadAnimEvent("../Asset/Json/Akaza_events.json", animations);
	jsonLoader.Free();


	auto ctrl = m_pAnimatorCom->GetAnimController();
	// 2) 상태(State) 등록
	   // Idle
	auto idleAnim = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseNut01_1");
	idleAnim->SetLoop(true);
	size_t idleIdx = ctrl->AddState("Idle", idleAnim, m_pModelCom->GetAnimationMap()[idleAnim->Get_Name()]);

	// Run
	auto runAnim = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseRun01_1");
	runAnim->SetLoop(true);
	size_t runIdx = ctrl->AddState("Run", runAnim, 1);

	// RunEnd (달리다 멈추는)  논루프
	auto runEndAnim = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseRun01_2");
	runEndAnim->SetLoop(false);
	size_t runEndIdx = ctrl->AddState("RunEnd", runEndAnim, 1);

	// Jump0~3
	vector<CAnimation*> jumpClips;
	for (int i = 0; i < 3; ++i)
	{
		auto name = "A_P1012_V00_C90_BaseJump01_" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name.c_str());
		a->SetLoop(false);
		jumpClips.push_back(a);
	}
	size_t jump0Idx = ctrl->AddState("Jump0", jumpClips[0], 2);
	size_t jump1Idx = ctrl->AddState("Jump1", jumpClips[1], 2);
	size_t jump3Idx = ctrl->AddState("Jump2", jumpClips[2], 2);
	jumpClips[1]->SetTickPerSecond(18.f); // 1번 점프 속도
	jumpClips[2]->SetTickPerSecond(35.f);

	vector<CAnimation*> comboAttackClips;
	for (int i = 1; i <= 4; ++i)
	{
		auto name = "A_P1012_V00_C90_AtkCmbW0" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name.c_str());
		a->SetLoop(false);
		comboAttackClips.push_back(a);
	}
	auto attackDown = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_AtkCmbW03D01");
	attackDown->SetLoop(false);
	comboAttackClips.push_back(attackDown);

	auto attackUp = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_AtkCmbW03U01");
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

	// Idle → Attack0 : 버튼 누르면 즉시 진입
	AniCon c0{ "Attack", CAnimController::EOp::Trigger, 0.f };
	c0.minTime = 0.1f; c0.maxTime = 1.f;
	ctrl->AddTransition(idleIdx, attack0Idx, c0, 0.1f);

	// Attack0 → Attack1 : 20~60% 구간에만
	AniCon c1{ "Attack", CAnimController::EOp::Trigger, 0.f };
	c1.minTime = 0.1f; c1.maxTime = 1.f;
	ctrl->AddTransition(attack0Idx, attack1Idx, c1, 0.1f);

	// Attack1 → Attack2 : 20~60% 구간
	AniCon c2{ "Attack", CAnimController::EOp::Trigger, 0.f };
	c2.minTime = 0.1f; c2.maxTime = 1.f;
	ctrl->AddTransition(attack1Idx, attack2Idx, c2, 0.1f);

	// Attack2 → Attack3 : 20~60% 구간
	AniCon c3{ "Attack", CAnimController::EOp::Trigger, 0.f };
	c3.minTime = 0.1f; c3.maxTime = 1.f;
	ctrl->AddTransition(attack2Idx, attack3Idx, c3, 0.1f);



	//A_P0000_V00_C00_Dmg01_F


	auto animHurtFront = m_pModelCom->GetAnimationClipByName("A_P0000_V00_C00_Dmg01_F");
	animHurtFront->SetLoop(false);
	size_t hurtFIdx = ctrl->AddState("Hurt_F", animHurtFront, 12);

	auto animHurtAirborne = m_pModelCom->GetAnimationClipByName("A_P0000_V00_C00_Dmg01A_F"); // 공중 상태에서 
	animHurtAirborne->SetLoop(false);
	size_t hurtAirborneIdx = ctrl->AddState("Hurt_Airborne", animHurtAirborne, m_pModelCom->GetAnimationMap()[animHurtAirborne->Get_Name()]);


	// A_P0000_V00_C00_DmgFall01_0
	vector<CAnimation*> fallClips;
	for (_int i = 0; i < 3; i++)
	{
		auto name = "A_P0000_V00_C00_DmgFall01_" + to_string(i);
		auto anim = m_pModelCom->GetAnimationClipByName(name.c_str());
		anim->SetLoop(false);
		fallClips.push_back(anim);
	}

	size_t fall0Idx = ctrl->AddState("Hurt_Fall0", fallClips[0], m_pModelCom->GetAnimationMap()[fallClips[0]->Get_Name()]);
	size_t fall1Idx = ctrl->AddState("Hurt_Fall1", fallClips[1], m_pModelCom->GetAnimationMap()[fallClips[1]->Get_Name()]);
	size_t fall2Idx = ctrl->AddState("Hurt_Fall2", fallClips[2], m_pModelCom->GetAnimationMap()[fallClips[2]->Get_Name()]);


	// 바운드 애니메이션 A_P0000_V00_C00_DmgBound01_0
	CAnimation* animBound = m_pModelCom->GetAnimationClipByName("A_P0000_V00_C00_DmgBound01_0");
	animBound->SetLoop(false);
	size_t boundIdx = ctrl->AddState("Hurt_Bound", animBound, m_pModelCom->GetAnimationMap()[animBound->Get_Name()]);


	// A_P1012_V00_C90_BaseGuard01_0


	vector<CAnimation*> guardClips;
	for (int i = 0; i < 3; ++i)
	{
		auto name = "A_P1012_V00_C90_BaseGuard01_" + to_string(i);
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
		auto name = "A_P1012_V00_C90_AtkUniqueAct01_" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name.c_str());
		a->SetLoop(false);
		guardSkillClips.push_back(a);
	}

	// A_P1012_V00_C90_AtkUniqueAct01_0 상승염천
	size_t guardSkill0Idx = ctrl->AddState("guardSkill0", guardSkillClips[0], 5);
	size_t guardSkill1Idx = ctrl->AddState("guardSkill1", guardSkillClips[1], 5);
	size_t guardSkill2Idx = ctrl->AddState("guardSkill2", guardSkillClips[2], 5);

	auto animStep = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseStepB01");
	animStep->SetLoop(false);
	size_t stepBackIdx = ctrl->AddState("stepBack", animStep, 7);

	auto animStep2 = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseStepF01");
	animStep2->SetLoop(false);
	size_t stepFrontIdx = ctrl->AddState("stepFront", animStep2, 7);

	auto animStep3 = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseStepL01");
	animStep3->SetLoop(false);
	size_t stepLeftIdx = ctrl->AddState("stepLeft0", animStep3, 7);

	auto animStep4 = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseStepL02");
	animStep4->SetLoop(false);
	size_t stepLeftIdx2 = ctrl->AddState("stepLeft1", animStep4, 7);

	auto animStep5 = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseStepR01");
	animStep5->SetLoop(false);
	size_t stepRightIdx = ctrl->AddState("stepRight", animStep5, 7);
	auto animStep6 = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseStepR02");
	animStep6->SetLoop(false);
	size_t stepRightIdx2 = ctrl->AddState("stepRight2", animStep6, 7);

	auto animStep7 = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseStepAR01");
	animStep7->SetLoop(false);
	size_t stepRightJumpIdx = ctrl->AddState("stepRightJump", animStep7, 7);

	auto animStep8 = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseStepAL01");
	animStep8->SetLoop(false);
	size_t stepLeftJumpIdx = ctrl->AddState("stepLeftJump", animStep8, 7);

	auto animStep9 = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseStepAB01");
	animStep9->SetLoop(false);
	size_t stepBackJumpIdx = ctrl->AddState("stepBackJump", animStep9, 7);

	auto animStep10 = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseStepAF01");
	animStep10->SetLoop(false);
	size_t stepFrontJumpIdx = ctrl->AddState("stepFrontJump", animStep10, 7);



	// A_P1012_V00_C90_AtkSkl02 파괴살 공식
	auto skillDefault = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_AtkSkl02_0");
	skillDefault->SetLoop(false);
	skillDefault->SetTickPerSecond(35.f); // 스킬 속도 조정
	size_t skillDefaultIdx = ctrl->AddState("skill0", skillDefault, 8);

	// A_P1012_V00_C90_AtkSkl03_0 파괴살 공식
	vector<CAnimation*> skill1Clips;

	for (auto i = 0; i < 4; i++)
	{
		auto name = "A_P1012_V00_C90_AtkSkl03_" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name);
		a->SetLoop(false);
		skill1Clips.push_back(a);
	}
	size_t skill1Idx0 = ctrl->AddState("skill1_0", skill1Clips[0], 10);
	size_t skill1Idx1 = ctrl->AddState("skill1_1", skill1Clips[1], 10);
	size_t skill1Idx2 = ctrl->AddState("skill1_2", skill1Clips[2], 10);
	size_t skill1EndIdx = ctrl->AddState("skill1End", skill1Clips[3], 10);




	// 3) 파라미터(Parameter) 등록
	m_pAnimatorCom->AddBool("Move");
	m_pAnimatorCom->AddBool("Jump");
	m_pAnimatorCom->AddBool("Guard");
	m_pAnimatorCom->AddBool("Death");
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
	m_pAnimatorCom->AddTrigger("HurtAir");
	m_pAnimatorCom->AddTrigger("HurtBound");
	m_pAnimatorCom->AddTrigger("HurtBlow");


	// 추적 대시 A_P1012_V00_C90_AtkSkl01
	auto dashAttackAnim = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_AtkSkl01_Cut");
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

	//A_P1012_V00_C90_AtkCmbAW01 점프 중 공격

	auto jumpAttack = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_AtkCmbAW01");
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

	// 공식
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

	// 난식
	CAnimController::Condition cBreath{ "Skill1", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(attack0Idx, skill1Idx0, cBreath);
	ctrl->AddTransition(attack1Idx, skill1Idx0, cBreath);
	ctrl->AddTransition(attack2Idx, skill1Idx0, cBreath);
	ctrl->AddTransition(attack3Idx, skill1Idx0, cBreath);
	ctrl->AddTransition(attack4Idx, skill1Idx0, cBreath);
	ctrl->AddTransition(attack5Idx, skill1Idx0, cBreath);
	ctrl->AddTransition(runEndIdx, skill1Idx0, cBreath);
	ctrl->AddTransition(skill1Idx0, skill1Idx1, cFin, 0.1f);
	ctrl->AddTransition(skill1Idx1, skill1Idx2, cFin, 0.1f);
	ctrl->AddTransition(skill1Idx2, skill1EndIdx, cFin, 0.1f);
	ctrl->AddTransition(skill1EndIdx, runIdx, cSpeedUp);
	ctrl->AddTransition(skill1EndIdx, idleIdx, cFin);

	// 나침
	CAnimController::Condition cRisingSun{ "Skill2", CAnimController::EOp::Trigger, 0.f };
	//ctrl->AddTransition(attack0Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack1Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack2Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack3Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack4Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack5Idx, skillDefaultIdx, cRisingSun);
	ctrl->AddTransition(guardSkill0Idx, guardSkill1Idx, cFin, 0.1f);
	ctrl->AddTransition(guardSkill1Idx, guardSkill2Idx, cFin, 0.1f);
	ctrl->AddTransition(guard0Idx, guardSkill0Idx, cRisingSun, 0.1f);
	ctrl->AddTransition(guard1Idx, guardSkill0Idx, cRisingSun, 0.1f);
	ctrl->AddTransition(guard2Idx, guardSkill0Idx, cRisingSun, 0.1f);
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
	ctrl->AddTransition(hurtFIdx, hurtFIdx, cHurt, 0.3f);

	ctrl->AddTransition(attack0Idx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(attack1Idx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(attack2Idx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(attack3Idx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(attack4Idx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(attack5Idx, hurtFIdx, cHurt, 0.1f);

	ctrl->AddTransition(hurtFIdx, idleIdx, cFin);
	ctrl->AddTransition(hurtFIdx, runIdx, cSpeedUp, 0.1f);

	CAnimController::Condition cHurtAir{ "HurtAir", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(runIdx, hurtAirborneIdx, cHurtAir, 0.1f);
	ctrl->AddTransition(runEndIdx, hurtAirborneIdx, cHurtAir, 0.1f);
	ctrl->AddTransition(idleIdx, hurtAirborneIdx, cHurtAir, 0.1f);
	ctrl->AddTransition(hurtAirborneIdx, hurtAirborneIdx, cHurtAir, 0.5f);


	ctrl->AddTransition(attack0Idx, hurtAirborneIdx, cHurtAir, 0.1f);
	ctrl->AddTransition(attack1Idx, hurtAirborneIdx, cHurtAir, 0.1f);
	ctrl->AddTransition(attack2Idx, hurtAirborneIdx, cHurtAir, 0.1f);
	ctrl->AddTransition(attack3Idx, hurtAirborneIdx, cHurtAir, 0.1f);
	ctrl->AddTransition(attack4Idx, hurtAirborneIdx, cHurtAir, 0.1f);
	ctrl->AddTransition(attack5Idx, hurtAirborneIdx, cHurtAir, 0.1f);
	ctrl->AddTransition(hurtAirborneIdx, idleIdx, cFin);
	ctrl->AddTransition(hurtAirborneIdx, runIdx, cSpeedUp, 0.1f);

	AniCon cHurtBound{ "HurtBound", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(runIdx, boundIdx, cHurtBound, 0.1f);
	ctrl->AddTransition(runEndIdx, boundIdx, cHurtBound, 0.1f);
	ctrl->AddTransition(idleIdx, boundIdx, cHurtBound, 0.1f);

	ctrl->AddTransition(attack0Idx, boundIdx, cHurtBound, 0.1f);
	ctrl->AddTransition(attack1Idx, boundIdx, cHurtBound, 0.1f);
	ctrl->AddTransition(attack2Idx, boundIdx, cHurtBound, 0.1f);
	ctrl->AddTransition(attack3Idx, boundIdx, cHurtBound, 0.1f);
	ctrl->AddTransition(attack4Idx, boundIdx, cHurtBound, 0.1f);
	ctrl->AddTransition(attack5Idx, boundIdx, cHurtBound, 0.1f);
	ctrl->AddTransition(stepBackIdx, boundIdx, cHurtBound, 0.1f);
	ctrl->AddTransition(stepFrontIdx, boundIdx, cHurtBound, 0.1f);
	ctrl->AddTransition(stepLeftIdx, boundIdx, cHurtBound, 0.1f);
	ctrl->AddTransition(stepRightIdx, boundIdx, cHurtBound, 0.1f);

	// 공중에서 혹시나 공격받으면 공중 히트 애니메이션으로 전이
	ctrl->AddTransition(boundIdx, hurtAirborneIdx, cHurtAir, 0.1f);
	ctrl->AddTransition(fall0Idx, hurtAirborneIdx, cHurtAir, 0.1f);
	ctrl->AddTransition(fall1Idx, hurtAirborneIdx, cHurtAir, 0.1f);
	ctrl->AddTransition(fall2Idx, hurtAirborneIdx, cHurtAir, 0.1f);

	ctrl->AddTransition(boundIdx, fall0Idx, cFin, 0.1f);
	ctrl->AddTransition(fall0Idx, fall1Idx, cFin, 0.1f);
	ctrl->AddTransition(fall1Idx, fall2Idx, cFin, 0.1f);

	ctrl->AddTransition(fall2Idx, idleIdx, cFin);
	ctrl->AddTransition(fall2Idx, runIdx, cSpeedUp, 0.1f);

	// Blow 는 Fall 애니메이션

	CAnimController::Condition cHurtBlow{ "HurtBlow", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(runIdx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(runEndIdx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(idleIdx, boundIdx, cHurtBlow, 0.1f);
	ctrl->AddTransition(hurtFIdx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(skill1Idx0, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(skill1EndIdx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(skillDefaultIdx, fall0Idx, cHurtBlow, 0.1f);

	ctrl->AddTransition(attack0Idx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(attack1Idx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(attack2Idx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(attack3Idx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(attack4Idx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(attack5Idx, fall0Idx, cHurtBlow, 0.1f);



}


_bool CAkaza::IsCooldownReady(_float& fCooldownVar, _float fTimeDelta, _float fBaseDelay)
{
	if (fCooldownVar > 0.f)
	{
		fCooldownVar -= fTimeDelta;
		return false;
	}
	fCooldownVar = fBaseDelay;
	return true;
}

void CAkaza::FillInput(InputData& outInput)
{
	CBaseCharacter* pTarget = dynamic_cast<CBaseCharacter*>(Get_Target());
	if (!pTarget)
		return;

	if (m_pAnimatorCom->CheckBool("Hurted"))
		return; // 피격 중이면 입력 무시

	float dt = CGameInstance::Get_Instance()->Get_TimeDelta(TEXT("Timer_60"));
	auto anim = m_pAnimatorCom; // 예: 애니메이터 컴포넌트 포인터

	// 3) 거리/방향 계산 (XZ 평면)
	XMVECTOR myPos = GetTransform()->Get_State(STATE::POSITION);
	XMVECTOR tgtPos = pTarget->GetTransform()->Get_State(STATE::POSITION);
	myPos = XMVectorSetY(myPos, 0.f);
	tgtPos = XMVectorSetY(tgtPos, 0.f);
	XMVECTOR diff = tgtPos - myPos;
	float dist = XMVectorGetX(XMVector3Length(diff));
	XMVECTOR dirToPlayer = (dist > 0.001f) ? XMVector3Normalize(diff) : XMVectorZero();

	// 4) 플레이어 상태 한 번에 확인 → 즉시 액션 결정
	CBaseCharacter::CSTATE playerState = pTarget->GetState();

	outInput = InputData();  // 기본값

	//if (!XMVector3Equal(dirToPlayer, XMVectorZero()))
	//	outInput.moveDir = XMVector3Normalize(dirToPlayer);
	auto commands = m_pInputBuffer->GetCommands();

	if (m_pInputBuffer->CheckCombo(commands,
		{ ECommand::LightAttack, ECommand::LightAttack,
		  ECommand::LightAttack, ECommand::LightAttack },
		1.f))
	{
		if (m_Distribution(m_RandGen) < 0.3f)
		{
			outInput.doAttack3Up = true;
		}
		else if (m_Distribution(m_RandGen) < 0.5f)
		{
			outInput.doAttack3Down = true;
		}

		outInput.doAttack4 = true;
		outInput.doAttack3 = true;
		outInput.doAttack2 = true;
		outInput.doAttack = true;

	}
	// 3타: LightAttack×3 + 위/아래 판정 (시간 제한 1초)
	else if (m_pInputBuffer->CheckCombo(commands,
		{ ECommand::LightAttack, ECommand::LightAttack,
		  ECommand::LightAttack },
		1.f))
	{
		outInput.doAttack3 = true;
		outInput.doAttack2 = true;
		outInput.doAttack = true;
	}
	else if (m_pInputBuffer->CheckCombo(commands,
		{ ECommand::LightAttack, ECommand::LightAttack },
		0.5f))
	{
		outInput.doAttack2 = true;
		outInput.doAttack = true;
	}
	else if (m_pInputBuffer->CheckCombo(commands,
		{ ECommand::LightAttack },
		0.2f))
	{
		outInput.doAttack = true;
	}


	for (const auto& cmd : commands)
	{
		switch (cmd.type)
		{
		case ECommand::Jump:
			outInput.doJump = true;
			break;
		case ECommand::Dash:
			outInput.doStep = true; // 또는 doDash 로 따로 관리
			{
				// 상대가 가드 중인지 체크
				_bool targetIsGuarding = (pTarget->GetState() == CBaseCharacter::CSTATE::GUARD || pTarget->GetState() == CBaseCharacter::CSTATE::MOVE||
					pTarget->GetState() == CBaseCharacter::CSTATE::IDLE) ;

				if (targetIsGuarding)
				{
					outInput.moveDir = XMVector3Normalize(dirToPlayer);
				}
				else if (outInput.doAttack)
				{
					// 공격 중이면 앞으로 대시
					outInput.moveDir = XMVector3Normalize(dirToPlayer);
				}
				else
				{
					// 그 외에는 뒤로 대시
					outInput.moveDir = XMVectorNegate(XMVector3Normalize(dirToPlayer));
				}
			}
			break;
		case ECommand::Guard:
			outInput.doGuard = true;
			break;
		case ECommand::Move:
			outInput.moveDir = dirToPlayer; // 이동 방향 설정
			break;
		case ECommand::Skill0:
			_bool bMoving = !XMVector3Equal(outInput.moveDir, XMVectorZero());
			if (bMoving)
			{
				outInput.doSkill1 = true;
			}
			else
			{
				outInput.doSkill0 = true;
			}
			break;
		}
	}
}

void CAkaza::HandleInput()
{
	CBaseCharacter* pTarget = dynamic_cast<CBaseCharacter*>(Get_Target());
	if (!pTarget)
		return;

	float dt = CGameInstance::Get_Instance()->Get_TimeDelta(TEXT("Timer_60"));

	// 0) 모든 쿨다운 타이머 감소
	if (m_fFollowCooldown > 0.f) m_fFollowCooldown -= dt;
	if (m_fAttackCooldown > 0.f) m_fAttackCooldown -= dt;
	if (m_fGuardCooldown > 0.f) m_fGuardCooldown -= dt;
	if (m_fStepCooldown > 0.f) m_fStepCooldown -= dt;
	if (m_fJumpCooldown > 0.f) m_fJumpCooldown -= dt;

	// 1) 거리/방향 계산 (XZ 평면)
	XMVECTOR myPos = GetTransform()->Get_State(STATE::POSITION);
	XMVECTOR tgtPos = pTarget->GetTransform()->Get_State(STATE::POSITION);
	myPos = XMVectorSetY(myPos, 0.f);
	tgtPos = XMVectorSetY(tgtPos, 0.f);
	XMVECTOR diff = tgtPos - myPos;
	float    dist = XMVectorGetX(XMVector3Length(diff));
	XMVECTOR dirToPlayer = (dist > 0.001f) ? XMVector3Normalize(diff) : XMVectorZero();

	CBaseCharacter::CSTATE playerState = pTarget->GetState();


	if (m_pGameInstance->IsKeyPressed(VK_TAB))
	{
		int a = 0;
	}
	// 4-1) 플레이어 IDLE 상태
	if (playerState == CBaseCharacter::CSTATE::IDLE)
	{
		// (1) 공격 범위 이내 공격 시도
		if (dist <= 25.f)
		{
			if (m_fAttackCooldown <= 0.f)
			{
				m_pInputBuffer->AddCommand({ ECommand::LightAttack, m_fTotalTime });
				m_fAttackCooldown = 0.1f; // 공격 쿨다운 예시
			}
			return;
		}	
		
		if (dist >= 70.f)
		{
			m_pInputBuffer->AddCommand({ ECommand::Dash, m_fTotalTime });
		}
		

		return;
	}
	// 4-2) 플레이어 MOVE 상태
	else if (playerState == CBaseCharacter::CSTATE::MOVE)
	{
		//// (1) 공격 범위 이내 → 공격
		if (dist <= 25.f)
		{
			if (m_fAttackCooldown <= 0.f)
			{
				m_pInputBuffer->AddCommand({ ECommand::LightAttack, m_fTotalTime });
				m_fAttackCooldown = 0.1f;
			}
			return;
		}

		// (2) 매우 멀리 있으면 자동 추격 (거리 예: dist 20)
		if (dist >= 40.f&&dist<60.f)
		{
			m_pInputBuffer->AddCommand({ ECommand::Move, m_fTotalTime });
			return;
		}
		else if (dist >= 60.f)
		{
			if (m_fStepCooldown <= 0.f)
			{
				m_pInputBuffer->AddCommand({ ECommand::Dash, m_fTotalTime });
				m_fStepCooldown = 3.0f; // 대시 쿨다운 예시
			}
		}

		return;
	}
	// 4-3) 플레이어 JUMP 상태
	else if (playerState == CBaseCharacter::CSTATE::JUMP)
	{
		m_pInputBuffer->AddCommand({ ECommand::Skill0, m_fTotalTime });
		//if (m_fJumpCooldown <= 0.f)
		//{
		//	m_pInputBuffer->AddCommand({ ECommand::Jump, m_fTotalTime });
		//	m_fJumpCooldown = 1.5f;
		//}
		return;
	}
	// 4-4) 플레이어 ATTACK 상태
	else if (playerState == CBaseCharacter::CSTATE::ATTACK
		|| playerState == CBaseCharacter::CSTATE::ATTACK2|| playerState == CBaseCharacter::CSTATE::ATTACK3
		|| playerState == CBaseCharacter::CSTATE::ATTACK4)
	{
		// (1) 랜덤 확률로 가드하기
		if (m_Distribution(m_RandGen) < 0.55f)
		{
			if (m_fGuardCooldown <= 0.f)
			{
				m_pInputBuffer->AddCommand({ ECommand::Guard, m_fTotalTime });
				m_fGuardCooldown = 3.f; // 가드 쿨다운 예시
			}
			if (dist >= 30.f)
			{
				m_pInputBuffer->AddCommand({ ECommand::Move, m_fTotalTime });
			}
			return;
		}
		// (2) 아니면 뒤로 대시(Back-Dash)
		else
		{
			if (m_fStepCooldown <= 0.f)
			{
				m_pInputBuffer->AddCommand({ ECommand::Dash, m_fTotalTime });
				m_fStepCooldown = 5.0f;
			}
			return;
		}
	}
	// 4-5) 플레이어 GUARD 상태
	else if (playerState == CBaseCharacter::CSTATE::GUARD)
	{
		auto r = m_Distribution(m_RandGen);
		if (r < 0.55f)
		{
			if (dist <= 25.f)
			{
				if (m_fAttackCooldown <= 0.f)
				{
					m_pInputBuffer->AddCommand({ ECommand::LightAttack, m_fTotalTime });
					m_fAttackCooldown = 0.2f;
				}
				return;
			}
			else if (dist >= 40.f)
			{
				m_pInputBuffer->AddCommand({ ECommand::Move, m_fTotalTime });
				return;
			}
			else
			{
				m_pInputBuffer->AddCommand({ ECommand::Skill1, m_fTotalTime });
				return;
			}
		}
		else
		{
			// r >= 0.55f 이므로, 남은 확률은 0.45 (45%)
		// 이 45%를 다시 대시 vs 가드로 나눌 때:
			//   - 대시 30% (0.30), 가드 15% (0.15) 이런 식
				//   비교값을 0.55 + 0.30 = 0.85

			if (r < 0.85f)
			{
				if (m_fGuardCooldown <= 0.f)
				{
					m_pInputBuffer->AddCommand({ ECommand::Guard, m_fTotalTime });
					m_fGuardCooldown = 1.0f;
				}

			}
			else
			{
				// 85~100(=1.0) 사이면: 15%
				if (m_fStepCooldown <= 0.f)
				{
					m_pInputBuffer->AddCommand({ ECommand::Dash, m_fTotalTime });
					m_fStepCooldown = 4.0f;
				}
			}
		}

		return;
	}
	// 4-6) 플레이어 SKILL 상태
	else if (playerState == CBaseCharacter::CSTATE::SKILL
		|| playerState == CBaseCharacter::CSTATE::SKILL1
		|| playerState == CBaseCharacter::CSTATE::SKILL2)
	{
		auto r = m_Distribution(m_RandGen);
		if (r < 0.55f)
		{
			if (dist <= 25.f)
			{
				if (m_fAttackCooldown <= 0.f)
				{
					m_pInputBuffer->AddCommand({ ECommand::LightAttack, m_fTotalTime });
					m_fAttackCooldown = 0.2f;
				}
				return;
			}
			else if (dist >= 40.f)
			{
				m_pInputBuffer->AddCommand({ ECommand::Move, m_fTotalTime });
				return;
			}
		}
		else
		{
			// r >= 0.55f 이므로, 남은 확률은 0.45 (45%)
		// 이 45%를 다시 대시 vs 가드로 나눌 때:
			//   - 대시 30% (0.30), 가드 15% (0.15) 이런 식
				//   비교값을 0.55 + 0.30 = 0.85

			if (r < 0.85f)
			{
				if (m_fGuardCooldown <= 0.f)
				{
					m_pInputBuffer->AddCommand({ ECommand::Guard, m_fTotalTime });
					m_fGuardCooldown = 1.0f;
				}

			}
			else
			{
				// 85~100(=1.0) 사이면: 15%
				if (m_fStepCooldown <= 0.f)
				{
					m_pInputBuffer->AddCommand({ ECommand::Dash, m_fTotalTime });
					m_fStepCooldown = 1.0f;
					m_fFollowCooldown = 0.7f;
				}
			}
		}
		return;
	}
}

void CAkaza::ActiveCollider()
{
	for (auto& pBodyColl : m_vecChildren)
	{
		if (dynamic_cast<CBodyColliderParts*>(pBodyColl))
		{
			pBodyColl->SetActive(true);
		}
	}
}

void CAkaza::DeactiveCollider()
{
	for (auto& pBodyColl : m_vecChildren)
	{
		if (dynamic_cast<CBodyColliderParts*>(pBodyColl))
		{
			pBodyColl->SetActive(false);
		}
	}
}


CAkaza* CAkaza::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAkaza* pInstance = new CAkaza(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAkaza");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CAkaza::Clone(void* pArg)
{
	CAkaza* pInstance = new CAkaza(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CAkaza");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAkaza::Free()
{
	__super::Free();
}
