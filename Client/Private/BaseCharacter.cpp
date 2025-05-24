#include "BaseCharacter.h"
#include "GameInstance.h"
#include "Animation.h"
#include "StateIdle.h"
#include "InputBuffer.h"
#include "Weapon.h"	

using AniCon = CAnimController::Condition;
CBaseCharacter::CBaseCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}

CBaseCharacter::CBaseCharacter(const CBaseCharacter& Prototype)
	: CGameObject(Prototype)
	, m_pShaderCom{ Prototype.m_pShaderCom }
	, m_pModelCom{ Prototype.m_pModelCom }
	, m_pColliderCom{ Prototype.m_pColliderCom }
	, m_pAnimatroCom{ Prototype.m_pAnimatroCom }
	, m_fMaxHP{ Prototype.m_fMaxHP }
	, m_fCurrentHP{ Prototype.m_fCurrentHP }
	, m_fStamina{ Prototype.m_fStamina }
	, m_pWeapon{ Prototype.m_pWeapon }
{
}

HRESULT CBaseCharacter::Initialize_Prototype()
{
	Ready_Components();
	return S_OK;
}

HRESULT CBaseCharacter::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};

	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 40.f;
	Desc.strName = TEXT("BaseCharacter");

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(0.1f, 0.1f, 0.1f));

	Ready_Animation();


	CGameObject* pWeapon = m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::GAMEPLAY), TEXT("Weapon"));

	Set_Weapon("R_Hand_1", dynamic_cast<CWeapon*>(pWeapon));

	if (m_pWeapon)
	{
		m_pWeapon->SetParent(this);
	}

	m_pInputBuffer = CInputBuffer::Create();

	if (m_pInputBuffer == nullptr)
		return E_FAIL;

	ChangeState(new StateIdle());

	CTransform* pTrans = m_pTransformCom;
	_vector pos = pTrans->Get_State(STATE::POSITION);

	// Convert _vector to XMFLOAT3
	XMFLOAT3 posFloat3;
	XMStoreFloat3(&posFloat3, pos);
	// Quaternion rot = pTrans->GetRotation(); // 회전도 필요하면
	PxTransform physxT
	{
		{ posFloat3.x, posFloat3.y, posFloat3.z },
		//{ rot.x, rot.y, rot.z, rot.w } 
	};

	// PhysX 매니저에 만든 헬퍼를 사용해서 Static Actor 생성

	PxRigidActor* pActor =  m_pGameInstance->CreateRigidStatic(physxT);

	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_CapsuleCollider"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), pActor)))
		return E_FAIL;
	return S_OK;
}

void CBaseCharacter::Priority_Update(_float fTimeDelta)
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
}

void CBaseCharacter::Update(_float fTimeDelta)
{

     m_fTotalTime += fTimeDelta;
	m_pInputBuffer->Update(m_fTotalTime);
	 HandleInput();


	// 3) 애니메이션 업데이트
	m_pAnimatroCom->GetAnimController()->Update(fTimeDelta);
	m_pModelCom->Play_Animation(fTimeDelta);

	if (m_pState)
	{
		m_pState->Update(this, fTimeDelta);
	}


	if (m_pAnimatroCom)
	{
		const char* cur = m_pAnimatroCom->GetCurrentAnimName();
		char buf[MAX_PATH];
		sprintf_s(buf, "현재 애니메이션: %s", cur);
		SetWindowTextA(g_hWnd, buf);
	}


}

void CBaseCharacter::Late_Update(_float fTimeDelta)
{
	CGameObject::Late_Update(fTimeDelta);

	//_float3 tmp{};  
	//XMStoreFloat3(&tmp, m_pTransformCom->Get_State(STATE::POSITION));  
	//if (m_pGameInstance->IsAABBInFrustum(tmp, m_pTransformCom->Get_Scaled()))  
	//{  
	//  
	//}
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
}

HRESULT CBaseCharacter::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION))))
		return E_FAIL;


	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	//for (_uint i = 0; i < iNumMesh; i++)
	//{
	//	if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
	//		return E_FAIL;

	//	if (FAILED(m_pShaderCom->Begin(0)))
	//		return E_FAIL;

	//	if (FAILED(m_pModelCom->Render(i)))
	//		return E_FAIL;
	//}


	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

void CBaseCharacter::Set_Weapon(const char* boneName, CWeapon* pWeapon)
{
	m_pWeapon = pWeapon;

	CBone* pBoneRHand = m_pModelCom->Get_Bone(boneName);

	if (pBoneRHand && m_pWeapon)
	{
		m_pWeapon->Set_BoneSocket(pBoneRHand);
	}
}

void CBaseCharacter::ChangeState(IState* pState)
{
	if (m_pState)
	{
		m_pState->Exit(this);

	}
	if (pState)
	{
		pState->Enter(this);
	}
	Safe_Delete(m_pState);
	m_pState = pState;
}

void CBaseCharacter::HandleInput()
{
	if (m_pGameInstance->IsKeyPressed('J')) m_pInputBuffer->AddCommand({ ECommand::LightAttack, m_fTotalTime });
	if (m_pGameInstance->IsKeyPressed('K')) m_pInputBuffer->AddCommand({ ECommand::Jump, m_fTotalTime });
	if (m_pGameInstance->IsKeyPressed('L')) m_pInputBuffer->AddCommand({ ECommand::Dash, m_fTotalTime });
	if (m_pGameInstance->IsKeyPressed('O')) m_pInputBuffer->AddCommand({ ECommand::Guard, m_fTotalTime });
	if (m_pGameInstance->IsKeyPressed('I')) m_pInputBuffer->AddCommand({ ECommand::Skill0, m_fTotalTime });
}

void CBaseCharacter::Ready_Animation()
{
	m_pAnimatroCom->PlayClip(m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseNut01_1"));

	auto ctrl = m_pAnimatroCom->GetAnimController();
	// 2) 상태(State) 등록
	   // Idle
	auto idleAnim = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_BaseNut01_1");
	idleAnim->SetLoop(true);
	size_t idleIdx = ctrl->AddState("Idle", idleAnim, m_pModelCom->GetAnimationMap()[idleAnim->Get_Name()]);

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
	size_t jump2Idx = ctrl->AddState("Jump2", jumpClips[2], 2);
	size_t jump3Idx = ctrl->AddState("Jump3", jumpClips[3], 2);

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
	m_pAnimatroCom->AddTrigger("StepRight");


	// 추적 대시 A_P0012_V00_C00_AtkSkl01
	auto dashAttackAnim = m_pModelCom->GetAnimationClipByName("A_P0012_V00_C00_AtkSkl01");
	dashAttackAnim->SetLoop(false);
	size_t dashAttackIdx = ctrl->AddState("dashAttack", dashAttackAnim, 6);


	// Idle → Run 
	CAnimController::Condition cSpeedUp{ "Move", CAnimController::EOp::IsTrue, 0.1f };
	ctrl->AddTransition(idleIdx, runIdx, cSpeedUp, 0.1f);
	ctrl->AddTransition(runEndIdx, runIdx, cSpeedUp, 0.1f);
	// Run → RunEnd 
	CAnimController::Condition cSpeedDown{ "Move", CAnimController::EOp::IsFalse, 0.1f };

	ctrl->AddTransition(runIdx, runEndIdx, cSpeedDown, 0.2f);

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
	size_t jumpAttackIdx = ctrl->AddState("jumpAttack", jumpAttack, 9);
	CAnimController::Condition cJumpAttack{ "JumpAttack", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(jump0Idx, jumpAttackIdx, cJumpAttack, 0.1f);	
	ctrl->AddTransition(jump1Idx, jumpAttackIdx, cJumpAttack, 0.1f);	
	ctrl->AddTransition(jump2Idx, jumpAttackIdx, cJumpAttack, 0.1f);	
	ctrl->AddTransition(jump3Idx, jumpAttackIdx, cJumpAttack, 0.1f);	



	// Idle/Run → Jump0 : Jump Trigger
	CAnimController::Condition cJump{ "Jump", CAnimController::EOp::IsTrue, 0.f };
	ctrl->AddTransition(runEndIdx, jump0Idx, cJump);
	ctrl->AddTransition(runIdx, jump0Idx, cJump);
	ctrl->AddTransition(idleIdx, jump0Idx, cJump);

	// Jump0 → Jump1, Jump1 → Jump2, Jump2 → Jump3 : Finished
	ctrl->AddTransition(jump0Idx, jump1Idx, cFinished, 0.05f);
	ctrl->AddTransition(jump1Idx, jump2Idx, cFinished, 0.05f);
	ctrl->AddTransition(jump2Idx, jump3Idx, cFinished, 0.05f);
	// Jump0 -> JumpAttack ->Jump2->Jump3
	ctrl->AddTransition(jumpAttackIdx, jump2Idx, cFinished, 0.05f);

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

	CAnimController::Condition StepBack{ "StepBack", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(runIdx, stepBackIdx, StepBack, 0.1f);
	ctrl->AddTransition(stepBackIdx, idleIdx, cFin, 0.1f);

	CAnimController::Condition StepFront{ "StepFront", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(runIdx, stepFrontIdx, StepFront, 0.1f);
	ctrl->AddTransition(stepFrontIdx, idleIdx, cFin, 0.1f);

	CAnimController::Condition StepLeft{ "StepLeft", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(runIdx, stepLeftIdx, StepLeft, 0.1f);
	ctrl->AddTransition(stepLeftIdx, idleIdx, cFin, 0.1f);

	CAnimController::Condition StepRight{ "StepRight", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(runIdx, stepRightIdx, StepRight, 0.1f);
	ctrl->AddTransition(stepRightIdx, idleIdx, cFin, 0.1f);

}

HRESULT CBaseCharacter::Ready_Components()
{

	///* For.Com_Shader */
	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
	//	TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
	//	return E_FAIL;

	//if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
	//	return E_FAIL;

	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxAnimMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Fiona"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_AnimController*/
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Animator"),
		TEXT("Com_Animator"), reinterpret_cast<CComponent**>(&m_pAnimatroCom), m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

CBaseCharacter* CBaseCharacter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBaseCharacter* pInstance = new CBaseCharacter(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CBaseCharacter");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CBaseCharacter::Clone(void* pArg)
{
	CBaseCharacter* pInstance = new CBaseCharacter(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CBaseCharacter");
		Safe_Release(pInstance);
	}
	return pInstance;
}
void CBaseCharacter::Free()
{
	__super::Free();
	if (!m_bIsCloned)
	{
		Safe_Release(m_pShaderCom);
		Safe_Release(m_pModelCom);

		Safe_Release(m_pAnimatroCom);
	}
	Safe_Release(m_pColliderCom);
	Safe_Delete(m_pState);
	Safe_Release(m_pInputBuffer);
}
