#include "Tanjiro.h"
#include "GameInstance.h"
#include "Animation.h"
#include "StateIdle.h"
#include "InputBuffer.h"
#include "Weapon.h"	
#include "BodyColliderParts.h"
#include <JsonLoader.h>
#include "UIProgressBar.h"
#include <EnmuMeat.h>
#include <EnmuParts.h>
#include "Navigation.h"
#include "StateHurt.h"
#include "StateHurtAir.h"
#include "StateHurtBlow.h"

using AniCon = CAnimController::Condition;
CTanjiro::CTanjiro(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBaseCharacter(pDevice, pContext)
{

}

CTanjiro::CTanjiro(const CTanjiro& Prototype)
	: CBaseCharacter(Prototype)
{
}
HRESULT CTanjiro::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTanjiro::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};

	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 40.f;
	Desc.strName = TEXT("Tanjiro");



	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(0.1f, 0.1f, 0.1f));


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


	CGameObject* pWeapon = m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::ENMU_BOSS), TEXT("Weapon"));

		//Set_Weapon("R_Hand_1", dynamic_cast<CWeapon*>(pWeapon));
	Set_Weapon("R_Hand_1_Lct", dynamic_cast<CWeapon*>(pWeapon));

	if (m_pWeapon) // 자식벡터로 넣지는 않음
	{
		m_pWeapon->SetParent(this);
	//	m_pWeapon->GetTransform()->Scaling(_float3(10.f,10.f,10.f));
	}

	m_pColliderCom->SetListener(this);
	ChangeState(new StateIdle(TEXT("Idle")));
	m_iShaderPass = 2;

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, -18.f, -170.f, 1.f));

	if (m_pNavigationCom)
	{
	m_pNavigationCom->FindIndexCell(m_pTransformCom->Get_State(STATE::POSITION));
	}

	m_fGoroundHeight = -18.f; // 초기 지면 높이 설정

    return S_OK;
}

void CTanjiro::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
	if (!m_pTarget)
	{
		auto enmu = m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::ENMU_BOSS), TEXT("EnmuMeat")); 
		if (enmu)
		{
			auto pMeat = static_cast<CEnmuMeat*>(enmu);
			if (pMeat)
			{
				m_pTarget = static_cast<CEnmuParts*>(pMeat->GetPart(CEnmuMeat::BODY));
			}
		}
	}
}

void CTanjiro::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	for (auto& child : m_vecChildren)
	{
		child->Update(fTimeDelta);
	}
}

void CTanjiro::Late_Update(_float fTimeDelta)
{
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
}

HRESULT CTanjiro::Render()
{
#ifdef DEBUG
	if (m_pNavigationCom)
		m_pNavigationCom->Render();
#endif // DEBUG

	
	return __super::Render();
}

void CTanjiro::TakeDamage(_float fDamage)
{
	if (m_eState == CSTATE::DOWN)
	{
		__super::TakeDamage(fDamage);
		auto pBar = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("LeftLifeBar"));
		if (pBar)
		{
			CUIProgressBar* pRightBar = static_cast<CUIProgressBar*>(pBar);
			pRightBar->ApplyDamage(fDamage);
		}
		return;
	}
	if (m_eState == CSTATE::GUARD)
	{
		fDamage *= 0.5f; // 가드 중에는 피해량 감소
	}
	if (m_eState == CSTATE::SKILL2)
		return;
	__super::TakeDamage(fDamage);
	if (!m_bAirborne && !m_bIsBound)
	{
		ChangeState(new StateHurt());
	}
	auto pBar = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("LeftLifeBar"));
	if (pBar)
	{
		CUIProgressBar* pRightBar = static_cast<CUIProgressBar*>(pBar);
		pRightBar->ApplyDamage(fDamage);
	}
}

void CTanjiro::OnAttackHit(CGameObject* pTarget)
{
	if (auto pBoss = dynamic_cast<CEnmuMeat*>(pTarget))
	{
		_bool bIsOpen = pBoss->GetState() == EnmuState::OPEN;
		switch (m_eState)
		{
		case CSTATE::ATTACK:
			bIsOpen ? pBoss->Hit(3.f) : pBoss->Hit(4.5f);
			break;
		case CSTATE::ATTACK2:
			bIsOpen ? pBoss->Hit(4.f) : pBoss->Hit(5.f);
			break;
		case CSTATE::ATTACK3:
			bIsOpen ? pBoss->Hit(3.f) : pBoss->Hit(4.5f);
			break;
		case CSTATE::ATTACK4:
			pBoss->Hit(5.f);
			break;
		case CSTATE::ATTACK_DOWN:
			pBoss->Hit(5.f);
			break;
		case CSTATE::ATTACK_UP:
			pBoss->Hit(5.f);
			break;
		case CSTATE::SKILL:
			pBoss->Hit(10.f);
			break;
		case CSTATE::SKILL1:
			pBoss->Hit(20.f);
			break;
		case CSTATE::SKILL2:
			pBoss->Hit(30.f);
			break;
		default:
			break;
		}
	}
}

HRESULT CTanjiro::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_Toon"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_Tanjiro"),
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;


	/* For.Com_Navigation */
	CNavigation::NAVIGATION_DESC		NaviDesc{};
	NaviDesc.iIndex = 4;

	if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom), &NaviDesc)))
		return E_FAIL;

	//
	//
	return S_OK;
}

void CTanjiro::Ready_Animation()
{
	m_pAnimatorCom->PlayClip(m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_BaseNut01_1"));

	auto animations = m_pModelCom->GetAnimations();
	CJsonLoader jsonLoader;
	jsonLoader.LoadAnimEvent("../Asset/Json/Tanjiro_events.json", animations);
	jsonLoader.Free();


	auto ctrl = m_pAnimatorCom->GetAnimController();
	// 2) 상태(State) 등록
	   // Idle
	auto idleAnim = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_BaseNut01_1");
	idleAnim->SetLoop(true);
	size_t idleIdx = ctrl->AddState("Idle", idleAnim, m_pModelCom->GetAnimationMap()[idleAnim->Get_Name()]);
	idleAnim->AddEvent({ 0.f, "DeactiveHitbox" }); // 0 프레임에 Hitbox 활성화

	// Run
	auto runAnim = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_BaseRun01_1");
	runAnim->SetLoop(true);
	size_t runIdx = ctrl->AddState("Run", runAnim, m_pModelCom->GetAnimationMap()[runAnim->Get_Name()]);

	// RunEnd (달리다 멈추는)  논루프
	auto runEndAnim = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_BaseRun01_2");
	runEndAnim->SetLoop(false);
	size_t runEndIdx = ctrl->AddState("RunEnd", runEndAnim, m_pModelCom->GetAnimationMap()[runEndAnim->Get_Name()]);

	// Jump0~3
	vector<CAnimation*> jumpClips;
	for (int i = 0; i < 4; ++i)
	{
		auto name = "A_P0001_V00_C00_BaseJump01_" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name.c_str());
		a->SetLoop(false);
		jumpClips.push_back(a);
	}
	size_t jump0Idx = ctrl->AddState("Jump0", jumpClips[0],  m_pModelCom->GetAnimationMap()[jumpClips[0]->Get_Name()]);
	size_t jump1Idx = ctrl->AddState("Jump1", jumpClips[1],  m_pModelCom->GetAnimationMap()[jumpClips[1]->Get_Name()]);
	size_t jump3Idx = ctrl->AddState("Jump3", jumpClips[3],  m_pModelCom->GetAnimationMap()[jumpClips[3]->Get_Name()]);
	jumpClips[1]->SetTickPerSecond(18.f); // 1번 점프 속도
	jumpClips[3]->SetTickPerSecond(35.f);

	vector<CAnimation*> comboAttackClips;
	for (int i = 1; i <= 4; ++i)
	{
		auto name = "A_P0001_V00_C00_AtkCmbW0" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name.c_str());
		a->SetLoop(false);
		comboAttackClips.push_back(a);
	}
	auto attackDown = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_AtkCmbW03D01");
	attackDown->SetLoop(false);
	comboAttackClips.push_back(attackDown);

	auto attackUp = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_AtkCmbW03U01");
	attackUp->SetLoop(false);
	comboAttackClips.push_back(attackUp);


	comboAttackClips[2]->SetTickPerSecond(25.f); // 3타 공격 속도
	comboAttackClips[4]->SetTickPerSecond(20.f); // 아래 공격 속도
	comboAttackClips[5]->SetTickPerSecond(20.f); // 위 공격 속도
	size_t attack0Idx = ctrl->AddState("attack0", comboAttackClips[0],  m_pModelCom->GetAnimationMap()[comboAttackClips[0]->Get_Name()]);
	size_t attack1Idx = ctrl->AddState("attack1", comboAttackClips[1],  m_pModelCom->GetAnimationMap()[comboAttackClips[1]->Get_Name()]);
	size_t attack2Idx = ctrl->AddState("attack2", comboAttackClips[2],  m_pModelCom->GetAnimationMap()[comboAttackClips[2]->Get_Name()]);
	size_t attack3Idx = ctrl->AddState("attack3", comboAttackClips[3],  m_pModelCom->GetAnimationMap()[comboAttackClips[3]->Get_Name()]);
	size_t attack4Idx = ctrl->AddState("attackDown", comboAttackClips[4], m_pModelCom->GetAnimationMap()[comboAttackClips[4]->Get_Name()]);
	size_t attack5Idx = ctrl->AddState("attackUp", comboAttackClips[5], m_pModelCom->GetAnimationMap()[comboAttackClips[5]->Get_Name()]);

	comboAttackClips[5]->AddEvent({ 36.f , "DeactiveHitbox" }); // 36 프레임

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
	size_t hurtFIdx = ctrl->AddState("Hurt_F", animHurtFront, m_pModelCom->GetAnimationMap()[animHurtFront->Get_Name()]);
	
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


	// A_P0001_V00_C00_BaseGuard01_0


	vector<CAnimation*> guardClips;
	for (int i = 0; i < 3; ++i)
	{
		auto name = "A_P0001_V00_C00_BaseGuard01_" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name.c_str());

		if (i == 1)
			a->SetLoop(true);
		else
			a->SetLoop(false);
		guardClips.push_back(a);
	}

	guardClips[0]->SetTickPerSecond(40.f); // 0번은 빠르게
	size_t guard0Idx = ctrl->AddState("guard0", guardClips[0],  m_pModelCom->GetAnimationMap()[guardClips[0]->Get_Name()]);
	size_t guard1Idx = ctrl->AddState("guard1", guardClips[1],  m_pModelCom->GetAnimationMap()[guardClips[1]->Get_Name()]);
	size_t guard2Idx = ctrl->AddState("guard2", guardClips[2],  m_pModelCom->GetAnimationMap()[guardClips[2]->Get_Name()]);


	vector<CAnimation*> guardSkillClips;
	for (int i = 0; i < 3; ++i)
	{
		auto name = "A_P0001_V00_C00_AtkSkl03_" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name.c_str());
		a->SetLoop(false);
		guardSkillClips.push_back(a);
	}

	// A_P0001_V00_C00_AtkSkl03_ 비틀린 소용돌이
	size_t guardSkill0Idx = ctrl->AddState("guardSkill0", guardSkillClips[0],  m_pModelCom->GetAnimationMap()[guardSkillClips[0]->Get_Name()]);
	size_t guardSkill1Idx = ctrl->AddState("guardSkill1", guardSkillClips[1],  m_pModelCom->GetAnimationMap()[guardSkillClips[1]->Get_Name()]);
	size_t guardSkill2Idx = ctrl->AddState("guardSkill2", guardSkillClips[2],  m_pModelCom->GetAnimationMap()[guardSkillClips[2]->Get_Name()]);

	auto animStep = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_BaseStepB01");
	animStep->SetLoop(false);
	size_t stepBackIdx = ctrl->AddState("stepBack", animStep, m_pModelCom->GetAnimationMap()[animStep->Get_Name()]);

	auto animStep2 = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_BaseStepF01");
	animStep2->SetLoop(false);
	size_t stepFrontIdx = ctrl->AddState("stepFront", animStep2, m_pModelCom->GetAnimationMap()[animStep2->Get_Name()]);

	auto animStep3 = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_BaseStepL01");
	animStep3->SetLoop(false);
	size_t stepLeftIdx = ctrl->AddState("stepLeft0", animStep3, m_pModelCom->GetAnimationMap()[animStep3->Get_Name()]);

	auto animStep4 = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_BaseStepL02");
	animStep4->SetLoop(false);
	size_t stepLeftIdx2 = ctrl->AddState("stepLeft1", animStep4, m_pModelCom->GetAnimationMap()[animStep4->Get_Name()]);

	auto animStep5 = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_BaseStepR01");
	animStep5->SetLoop(false);
	size_t stepRightIdx = ctrl->AddState("stepRight", animStep5, m_pModelCom->GetAnimationMap()[animStep5->Get_Name()]);
	auto animStep6 = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_BaseStepR02");
	animStep6->SetLoop(false);
	size_t stepRightIdx2 = ctrl->AddState("stepRight2", animStep6, m_pModelCom->GetAnimationMap()[animStep6->Get_Name()]);

	auto animStep7 = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_BaseStepAR01");
	animStep7->SetLoop(false);
	size_t stepRightJumpIdx = ctrl->AddState("stepRightJump", animStep7, m_pModelCom->GetAnimationMap()[animStep7->Get_Name()]);

	auto animStep8 = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_BaseStepAL01");
	animStep8->SetLoop(false);
	size_t stepLeftJumpIdx = ctrl->AddState("stepLeftJump", animStep8, m_pModelCom->GetAnimationMap()[animStep8->Get_Name()]);

	auto animStep9 = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_BaseStepAB01");
	animStep9->SetLoop(false);
	size_t stepBackJumpIdx = ctrl->AddState("stepBackJump", animStep9, m_pModelCom->GetAnimationMap()[animStep9->Get_Name()]);

	auto animStep10 = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_BaseStepAF01");
	animStep10->SetLoop(false);
	size_t stepFrontJumpIdx = ctrl->AddState("stepFrontJump", animStep10, m_pModelCom->GetAnimationMap()[animStep10->Get_Name()]);



	// A_P0001_V00_C00_AtkSkl02 물방아
	vector<CAnimation*> skill0Clips;
	for (_int i = 0; i < 3; i++)
	{
		auto name = "A_P0001_V00_C00_AtkSkl02_" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name);
		a->SetLoop(false);
		skill0Clips.push_back(a);
	}
	size_t skillDefaultIdx0 = ctrl->AddState("skill10", skill0Clips[0],m_pModelCom->GetAnimationMap()[skill0Clips[0]->Get_Name()]);
	size_t skillDefaultIdx1 = ctrl->AddState("skill11", skill0Clips[1],m_pModelCom->GetAnimationMap()[skill0Clips[1]->Get_Name()]);
	size_t skillDefaultIdx2 = ctrl->AddState("skill12", skill0Clips[2],m_pModelCom->GetAnimationMap()[skill0Clips[2]->Get_Name()]);

	// A_P0001_V00_C00_AtkSkl04 용소

	auto skill0 = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_AtkSkl04");
	size_t skill0Idx = ctrl->AddState("skill0", skill0, m_pModelCom->GetAnimationMap()[skill0->Get_Name()]);


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
	m_pAnimatorCom->AddTrigger("HurtAir");
	m_pAnimatorCom->AddTrigger("HurtBound");
	m_pAnimatorCom->AddTrigger("HurtBlow");
	m_pAnimatorCom->AddTrigger("HurtDown");
	m_pAnimatorCom->AddTrigger("Hurt");
	m_pAnimatorCom->AddBool("Stepping"); // 스텝 중인지 여부
	m_pAnimatorCom->AddBool("Hurted");





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

	//A_P0001_V00_C00_AtkCmbAW01 점프 중 공격

	auto jumpAttack = m_pModelCom->GetAnimationClipByName("A_P0001_V00_C00_AtkCmbAW01");
	jumpAttack->SetLoop(false);
	jumpAttack->SetTickPerSecond(35.f); // 점프 공격 속도 조정
	size_t jumpAttackIdx = ctrl->AddState("jumpAttack", jumpAttack, m_pModelCom->GetAnimationMap()[jumpAttack->Get_Name()]);
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

	//  용소 기본 I 스킬
	CAnimController::Condition cFlameTiger{ "Skill0", CAnimController::EOp::Trigger, 0.f };

	ctrl->AddTransition(attack0Idx, skill0Idx, cFlameTiger);
	ctrl->AddTransition(attack1Idx, skill0Idx, cFlameTiger);
	ctrl->AddTransition(attack2Idx, skill0Idx, cFlameTiger);
	ctrl->AddTransition(attack3Idx, skill0Idx, cFlameTiger);
	ctrl->AddTransition(attack4Idx, skill0Idx, cFlameTiger);
	ctrl->AddTransition(attack5Idx, skill0Idx, cFlameTiger);
	ctrl->AddTransition(idleIdx, skill0Idx, cFlameTiger, 0.2f);
	ctrl->AddTransition(skill0Idx, runIdx, cSpeedUp);
	ctrl->AddTransition(skill0Idx, idleIdx, cFin);

	//물방아 상태 
	CAnimController::Condition cBreath{ "Skill1", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(attack0Idx, skillDefaultIdx0, cBreath);
	ctrl->AddTransition(attack1Idx, skillDefaultIdx0, cBreath);
	ctrl->AddTransition(attack2Idx, skillDefaultIdx0, cBreath);
	ctrl->AddTransition(attack3Idx, skillDefaultIdx0, cBreath);
	ctrl->AddTransition(attack4Idx, skillDefaultIdx0, cBreath);
	ctrl->AddTransition(attack5Idx, skillDefaultIdx0, cBreath);
	ctrl->AddTransition(runEndIdx, skillDefaultIdx0, cBreath);
	ctrl->AddTransition(skillDefaultIdx0, skillDefaultIdx1, cFin);
	ctrl->AddTransition(skillDefaultIdx1, skillDefaultIdx2, cFin);

	ctrl->AddTransition(skillDefaultIdx2, runIdx, cFin);
	ctrl->AddTransition(skillDefaultIdx2, idleIdx, cFin);

	// 비틀린 소용돌이
	CAnimController::Condition cRisingSun{ "Skill2", CAnimController::EOp::Trigger, 0.f };
	//ctrl->AddTransition(attack0Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack1Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack2Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack3Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack4Idx, skillDefaultIdx, cRisingSun);
	//ctrl->AddTransition(attack5Idx, skillDefaultIdx, cRisingSun);
	ctrl->AddTransition(guard0Idx, guardSkill0Idx, cRisingSun, 0.1f);
	ctrl->AddTransition(guard1Idx, guardSkill0Idx, cRisingSun, 0.1f);
	ctrl->AddTransition(guard2Idx, guardSkill0Idx, cRisingSun, 0.1f);
	ctrl->AddTransition(guardSkill0Idx, guardSkill1Idx, cFin, 0.1f);
	ctrl->AddTransition(guardSkill1Idx, guardSkill2Idx, cFin, 0.1f);
	ctrl->AddTransition(guardSkill2Idx, runIdx, cSpeedUp);
	ctrl->AddTransition(guardSkill2Idx, idleIdx, cFin);




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
	ctrl->AddTransition(skill0Idx, hurtFIdx, cHurt, 0.1f);
	ctrl->AddTransition(skillDefaultIdx0, hurtFIdx, cHurt, 0.1f);

	ctrl->AddTransition(hurtFIdx, idleIdx, cFin);
	ctrl->AddTransition(hurtFIdx, runIdx, cSpeedUp, 0.1f);



	AniCon cHurtAir{ "HurtAir", CAnimController::EOp::Trigger, 0.f };
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

	AniCon cHurtBlow{ "HurtBlow", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(runIdx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(runEndIdx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(idleIdx, boundIdx, cHurtBlow, 0.1f);
	ctrl->AddTransition(hurtFIdx, fall0Idx, cHurtBlow, 0.1f);
	//ctrl->AddTransition(skill, fall0Idx, cHurtBlow, 0.1f);
	//ctrl->AddTransition(skill1EndIdx, fall0Idx, cHurtBlow, 0.1f);
	//ctrl->AddTransition(skillDefaultIdx, fall0Idx, cHurtBlow, 0.1f);

	ctrl->AddTransition(attack0Idx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(attack1Idx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(attack2Idx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(attack3Idx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(attack4Idx, fall0Idx, cHurtBlow, 0.1f);
	ctrl->AddTransition(attack5Idx, fall0Idx, cHurtBlow, 0.1f);

	AniCon cHurtDown{ "HurtDown", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(runIdx, fall2Idx, cHurtDown, 0.1f);
	ctrl->AddTransition(runEndIdx, fall2Idx, cHurtDown, 0.1f);
	ctrl->AddTransition(idleIdx, fall2Idx, cHurtDown, 0.1f);

	ctrl->AddTransition(attack0Idx, fall2Idx, cHurtDown, 0.1f);
	ctrl->AddTransition(attack1Idx, fall2Idx, cHurtDown, 0.1f);
	ctrl->AddTransition(attack2Idx, fall2Idx, cHurtDown, 0.1f);
	ctrl->AddTransition(attack3Idx, fall2Idx, cHurtDown, 0.1f);
	ctrl->AddTransition(attack4Idx, fall2Idx, cHurtDown, 0.1f);
	ctrl->AddTransition(attack5Idx, fall2Idx, cHurtDown, 0.1f);
	ctrl->AddTransition(stepBackIdx, fall2Idx, cHurtDown, 0.1f);
	ctrl->AddTransition(stepFrontIdx, fall2Idx, cHurtDown, 0.1f);
	ctrl->AddTransition(stepLeftIdx, fall2Idx, cHurtDown, 0.1f);
	ctrl->AddTransition(stepRightIdx, fall2Idx, cHurtDown, 0.1f);
	ctrl->AddTransition(fall2Idx, fall2Idx, cHurtDown, 0.2f);


}

void CTanjiro::ActiveCollider()
{
	if (m_pWeapon)
	{
		auto collider = static_cast<CSphereCollider*>(m_pWeapon->Get_Component(TEXT("Com_Collider")));
		auto collider1 = static_cast<CSphereCollider*>(m_pWeapon->Get_Component(TEXT("Com_Collider1")));
		auto collider2 = static_cast<CSphereCollider*>(m_pWeapon->Get_Component(TEXT("Com_Collider2")));

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
}

void CTanjiro::DeactiveCollider()
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
}

CTanjiro* CTanjiro::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTanjiro* pInstance = new CTanjiro(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTanjiro");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTanjiro::Clone(void* pArg)
{
	CTanjiro* pInstance = new CTanjiro(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CTanjiro");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTanjiro::Free()
{
	__super::Free();
}
