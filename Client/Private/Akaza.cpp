#include "Akaza.h"
#include "GameInstance.h"
#include "Animation.h"
#include "StateIdle.h"
#include "InputBuffer.h"
#include "StateMove.h"	
#include "StateAttack1.h"	
#include "StateJump.h"
#include "StateStep.h"
#include "StateGuard.h"
#include "StateSkill0.h"
#include "StateSkill1.h"
#include "StateSkill2.h"


using AniCon = CAnimController::Condition;
CAkaza::CAkaza(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBaseCharacter(pDevice, pContext)
{

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

	if (FAILED(CGameObject::Initialize(&Desc)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(0.1f, 0.1f, 0.1f));

	if (FAILED(Ready_Components()))
		return E_FAIL;

	Ready_Animation();


	Add_Component(TEXT("Com_CapsuleCollider"), CCapsuleCollider::Create(m_pDevice, m_pContext, 3.9f, 77.f), reinterpret_cast<CComponent**>(&m_pColliderCom));

	m_pColliderCom->Initialize(nullptr);
	m_pColliderCom->SetOffset(_float3(0.f, 8.1f, 0.f));

	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_CapsuleCollider"),
	//	TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom))))
	//	return E_FAIL;
	m_pColliderCom->SetListener(this);
	return S_OK;
}

void CAkaza::Priority_Update(_float fTimeDelta)
{
	static _uint iAnim = 0;
	if (m_pGameInstance->IsKeyPressed('N'))
	{
		m_pAnimatroCom->Set_Animation(iAnim, 0.15f);
		iAnim++;
	}

	if (m_pGameInstance->IsKeyPressed('M'))
	{
		m_pAnimatroCom->Set_Animation(iAnim, 0.15f);
		iAnim = max(0, iAnim - 1);
	}
	switch (m_eComState)
	{
	case COM_STATE::IDLE:
		break;
	case COM_STATE::MOVE:
		break;
	case COM_STATE::ATTACK:
		ChangeState(new StateAttack1());
		break;
	case COM_STATE::GUARD:
		ChangeState(new StateGuard());
		break;
	case COM_STATE::STEP:
		break;
	case COM_STATE::SKILL:
		break;
	case COM_STATE::JUMP:
		break;
	case COM_STATE::APPRACH:
		m_fFollowTime -= fTimeDelta;
		if (m_fFollowTime <= 0.f)
		{
			m_fFollowTime = 5.f;
			ChangeState(new StateIdle());
			m_eComState = COM_STATE::IDLE;
		}
		else
		{
			m_pTransformCom->Follow_Target(m_pTarget->GetTransform()->Get_State(STATE::POSITION), fTimeDelta, 10.f);
			m_pAnimatroCom->SetBool("Move", true);
			m_pTransformCom->LookAt(m_pTarget->GetTransform()->Get_State(STATE::POSITION));
		}
		break;
	case COM_STATE::HURT:
		break;

	}
}

void CAkaza::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CAkaza::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
	PredictPlayerState();
}

HRESULT CAkaza::Render()
{
	__super::Render();
	return S_OK;
}

HRESULT CAkaza::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxAnimMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Akaza"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_AnimController*/
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Animator"),
		TEXT("Com_Animator"), reinterpret_cast<CComponent**>(&m_pAnimatroCom), m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

void CAkaza::Ready_Animation()
{
	m_pAnimatroCom->PlayClip(m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_BaseNut01_1"));

	auto ctrl = m_pAnimatroCom->GetAnimController();
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




	//// 3타까지 위,아래 중 처리
	//// Attack2 ->AttackDown
	//ctrl->AddTransition(attack2Idx, attack4Idx, cAttackDown, 0.5f); // 3타가 끝나면 아래 공격으로

	////Attack2 -> AttackUp
	//ctrl->AddTransition(attack2Idx, attack5Idx, cAttackUp, 0.5f); // 3타가 끝나면 위 공격으로



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



	// A_P1012_V00_C90_AtkSkl02 염호
	auto skillDefault = m_pModelCom->GetAnimationClipByName("A_P1012_V00_C90_AtkSkl02A_0");
	skillDefault->SetLoop(false);
	skillDefault->SetTickPerSecond(35.f); // 스킬 속도 조정
	size_t skillDefaultIdx = ctrl->AddState("skill0", skillDefault, 8);

	// A_P1012_V00_C90_AtkSkl03_0 기염만상
	vector<CAnimation*> skill1Clips;

	for (auto i = 0; i < 2; i++)
	{
		auto name = "A_P1012_V00_C90_AtkSkl03_" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name);
		a->SetLoop(false);
		skill1Clips.push_back(a);
	}
	size_t skill1Idx = ctrl->AddState("skill1", skill1Clips[0], 10);
	size_t skill1EndIdx = ctrl->AddState("skill1End", skill1Clips[1], 10);


	// 3) 파라미터(Parameter) 등록
	m_pAnimatroCom->AddBool("Move");
	m_pAnimatroCom->AddBool("Jump");
	m_pAnimatroCom->AddBool("Guard");
	m_pAnimatroCom->AddBool("Attacking");
	m_pAnimatroCom->AddTrigger("JumpAttack");
	m_pAnimatroCom->AddTrigger("Attack");
	m_pAnimatroCom->AddTrigger("AttackDown");
	m_pAnimatroCom->AddTrigger("AttackUp");
	m_pAnimatroCom->AddTrigger("Skill0");
	m_pAnimatroCom->AddTrigger("Skill1");
	m_pAnimatroCom->AddTrigger("Skill2");
	m_pAnimatroCom->AddTrigger("DashAttack");
	m_pAnimatroCom->AddTrigger("StepBack");
	m_pAnimatroCom->AddTrigger("StepFront");
	m_pAnimatroCom->AddTrigger("StepLeft");
	m_pAnimatroCom->AddTrigger("StepLeft2");
	m_pAnimatroCom->AddTrigger("StepRight");
	m_pAnimatroCom->AddTrigger("StepRight2");
	m_pAnimatroCom->AddTrigger("StepRightJump");
	m_pAnimatroCom->AddTrigger("StepLeftJump");
	m_pAnimatroCom->AddTrigger("StepBackJump");
	m_pAnimatroCom->AddTrigger("StepFrontJump");
	m_pAnimatroCom->AddBool("Stepping"); // 스텝 중인지 여부


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

}

void CAkaza::PredictPlayerState()
{
	if (m_pTarget)
	{
		switch (m_pTarget->GetState())
		{
		case CBaseCharacter::CSTATE::IDLE:
			m_eComState = COM_STATE::ATTACK;
			break;
		case CBaseCharacter::CSTATE::MOVE:
			m_eComState = COM_STATE::APPRACH;
			break;
		case CBaseCharacter::CSTATE::JUMP:
				
		
			m_pAnimatroCom->SetBool("Attacking", false);
			m_pAnimatroCom->SetBool("Jump", true);
			break;
		case CBaseCharacter::CSTATE::ATTACK:
			m_eComState = COM_STATE::GUARD;
			break;
		case CBaseCharacter::CSTATE::GUARD:
			m_pInputBuffer->AddCommand({ ECommand::LightAttack, m_fTotalTime });
		//	m_pInputBuffer->AddCommand({ ECommand::Jump, m_fTotalTime });
		//	m_pInputBuffer->AddCommand({ ECommand::Dash, m_fTotalTime });
		//	m_pInputBuffer->AddCommand({ ECommand::Guard, m_fTotalTime });
		//	m_pInputBuffer->AddCommand({ ECommand::Skill0, m_fTotalTime });

			break;
		case CBaseCharacter::CSTATE::SKILL:
			break;
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
