#include "BaseCharacter.h"
#include "GameInstance.h"
#include "Animation.h"

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
	//_bool bGuard = m_pGameInstance->IsKeyDown('O');
	//m_pAnimatroCom->SetBool("Guard", bGuard);
	//if (!bGuard)
	//{

	//_bool bMoving = false;
	//_uint iState = 0;
	//if (m_pGameInstance->IsKeyDown(VK_UP))
	//{ 
	//	m_pTransformCom->Go_Straight(fTimeDelta); bMoving = true;
	//	iState = 1;
	//}
	//if (m_pGameInstance->IsKeyDown(VK_DOWN))
	//{
	//	//XMVECTOR axis = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	//	//_float    rotSpeed = m_pTransformCom->Get_RotationPerSec();    // 라디안/초
	//	//_float    dtForTurn = XM_PI / rotSpeed;                        // π 라디안 돌리는 데 걸리는 시간

	//	m_pTransformCom->Rotate_EulerAngles(_float3(0.f, 180.f, 0.f));
	//	iState = 2;
	//}
	//if (m_pGameInstance->IsKeyDown(VK_LEFT))
	//{ 
	//	m_pTransformCom->Go_Left(fTimeDelta); bMoving = true;
	//	m_pTransformCom->Rotate_EulerAngles(_float3(0.f, 90.f, 0.f));
	//	iState = 3;
	//}
	//if (m_pGameInstance->IsKeyDown(VK_RIGHT)) 
	//{ 
	//	m_pTransformCom->Go_Right(fTimeDelta); bMoving = true;
	//	m_pTransformCom->Rotate_EulerAngles(_float3(0.f, -90.f, 0.f));
	//	iState = 4;
	//}

	//// 2) 파라미터 세팅
	//m_pAnimatroCom->SetBool("Move", bMoving);

	//if (m_pAnimatroCom->CheckBool("Move")&&m_pGameInstance->IsKeyPressed('L'))
	//{
	//	if (iState==1)
	//		m_pAnimatroCom->SetTrigger("StepFront");
	//	else if (iState == 2)
	//		m_pAnimatroCom->SetTrigger("StepBack");
	//	else if (iState == 3)
	//		m_pAnimatroCom->SetTrigger("StepLeft");
	//	else if (iState == 4)
	//		m_pAnimatroCom->SetTrigger("StepRight");
	//}

	//if (m_pGameInstance->IsKeyPressed(VK_SPACE))
	//	m_pAnimatroCom->SetTrigger("Jump");

	//if (m_pGameInstance->IsKeyPressed('J'))
	//	m_pAnimatroCom->SetTrigger("Attack");

	//if (!bMoving)
	//{
	//	if (m_pGameInstance->IsKeyPressed('L'))
	//		m_pAnimatroCom->SetTrigger("DashAttack");
	//}
	//}
	//else
	//{
	//	if (m_pGameInstance->IsKeyPressed('I'))
	//		m_pAnimatroCom->SetTrigger("RisingScorchingSun");
	//}


	//if (m_pAnimatroCom)
	//	m_pAnimatroCom->GetAnimController()->Update(fTimeDelta);
	//m_pModelCom->Play_Animation(fTimeDelta);

		// 1) 가드 처리
	_bool bGuard = m_pGameInstance->IsKeyDown('O');
	m_pAnimatroCom->SetBool("Guard", bGuard);

	if (!bGuard)
	{
		// 2) 이동 방향 벡터 계산 (월드 기준이 아닌 로컬 축 사용 가능)
		XMVECTOR dir = XMVectorZero();
		if (m_pGameInstance->IsKeyDown(VK_UP))    dir += XMVectorSet(0.f, 0.f, 1.f, 0.f);
		if (m_pGameInstance->IsKeyDown(VK_DOWN))  dir += XMVectorSet(0.f, 0.f, -1.f, 0.f);
		if (m_pGameInstance->IsKeyDown(VK_LEFT))  dir += XMVectorSet(-1.f, 0.f, 0.f, 0.f);
		if (m_pGameInstance->IsKeyDown(VK_RIGHT)) dir += XMVectorSet(1.f, 0.f, 0.f, 0.f);

		bool bMoving = !XMVector3Equal(dir, XMVectorZero());
		m_pAnimatroCom->SetBool("Move", bMoving);

		if (bMoving&&!m_pAnimatroCom->CheckTrigger("Attack"))
		{
			// 2-1) 입력 방향으로 회전 (쿼터니언)
			m_pTransformCom->RotateToDirection(dir);

			m_pTransformCom->Go_Straight(fTimeDelta);

			if (m_pGameInstance->IsKeyPressed('L'))
			{
				_float dx = XMVectorGetX(dir);
				_float dz = XMVectorGetZ(dir);
				if (fabsf(dx) > fabsf(dz))
				{
					m_pAnimatroCom->SetTrigger(dx > 0 ? "StepRight" : "StepLeft");
				}
				else
				{
					m_pAnimatroCom->SetTrigger(dz > 0 ? "StepFront" : "StepBack");
				}
			}
		}
		else
		{
			// idle 상태에서 다른 트리거 처리 예: DashAttack
			if (m_pGameInstance->IsKeyPressed('L'))
				m_pAnimatroCom->SetTrigger("DashAttack");
		}

		// 점프/공격
		if (m_pGameInstance->IsKeyPressed(VK_SPACE))
			m_pAnimatroCom->SetTrigger("Jump");
		if (m_pGameInstance->IsKeyPressed('J'))
			m_pAnimatroCom->SetTrigger("Attack");
	}
	else
	{
		// 가드 스킬 처리
		if (m_pGameInstance->IsKeyPressed('I'))
			m_pAnimatroCom->SetTrigger("RisingScorchingSun");
	}

	// 3) 애니메이션 업데이트
	m_pAnimatroCom->GetAnimController()->Update(fTimeDelta);
	m_pModelCom->Play_Animation(fTimeDelta);


	if(m_pAnimatroCom)
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
	auto name = "A_P0012_V00_C00_AtkCmbW03D01";
	auto a = m_pModelCom->GetAnimationClipByName(name);
	a->SetLoop(false);
	comboAttackClips.push_back(a);

	size_t attack0Idx = ctrl->AddState("attack0", comboAttackClips[0], 3);
	size_t attack1Idx = ctrl->AddState("attack1", comboAttackClips[1], 3);
	size_t attack2Idx = ctrl->AddState("attack2", comboAttackClips[2], 3);
	size_t attack3Idx = ctrl->AddState("attack3", comboAttackClips[4], 3);
	size_t attack4Idx = ctrl->AddState("attack4", comboAttackClips[3], 3);

	// A_P0012_V00_C00_BaseGuard01_0


	vector<CAnimation*> guardClips;
	for (int i = 0; i < 3; ++i)
	{
		auto name = "A_P0012_V00_C00_BaseGuard01_" + to_string(i);
		auto a = m_pModelCom->GetAnimationClipByName(name.c_str());
		
		if(i==1)
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

	// A_P0012_V00_C00_AtkUniqueAct01_0
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


	// 3) 파라미터(Parameter) 등록
	m_pAnimatroCom->AddBool("Move");
	m_pAnimatroCom->AddTrigger("Jump");
	m_pAnimatroCom->AddTrigger("Attack");
	m_pAnimatroCom->AddBool("Guard");
	m_pAnimatroCom->AddTrigger("RisingScorchingSun");
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
	ctrl->AddTransition(idleIdx, runIdx, cSpeedUp, 0.2f);
	ctrl->AddTransition(runEndIdx, runIdx, cSpeedUp,0.1f);
	// Run → RunEnd 
	CAnimController::Condition cSpeedDown{ "Move", CAnimController::EOp::IsFalse, 0.1f };
	ctrl->AddTransition(runIdx, runEndIdx, cSpeedDown, 0.2f);

	// RunEnd → Idle : Finished (애니메이션 종료 시)
	CAnimController::Condition cFinished{ "", CAnimController::EOp::Finished, 0.f };
	ctrl->AddTransition(runEndIdx, idleIdx, cFinished);



	// Idle/Run → Jump0 : Jump Trigger
	CAnimController::Condition cJump{ "Jump", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, jump0Idx, cJump);
	ctrl->AddTransition(runIdx, jump0Idx, cJump);

	// Jump0 → Jump1, Jump1 → Jump2, Jump2 → Jump3 : Finished
	ctrl->AddTransition(jump0Idx, jump1Idx, cFinished, 0.05f);
	ctrl->AddTransition(jump1Idx, jump2Idx, cFinished, 0.05f);
	ctrl->AddTransition(jump2Idx, jump3Idx, cFinished, 0.05f);

	// Jump3 → Idle : Finished
	ctrl->AddTransition(jump3Idx, idleIdx, cFinished, 0.1f);

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
	c3.minTime = 0.2f; c3.maxTime = 0.6f;
	ctrl->AddTransition(attack2Idx, attack3Idx, c3, 0.1f);

	// Attack3 → Attack4 : 20~60% 구간
	AniCon c4{ "Attack", CAnimController::EOp::Trigger, 0.f };
	c4.minTime = 0.6f; c4.maxTime = 1.f;
	ctrl->AddTransition(attack3Idx, attack4Idx, c4, 0.5f);

	CAnimController::Condition cFin{ "", CAnimController::EOp::Finished, 0.f };

	// Attack0 → Idle
	ctrl->AddTransition(attack0Idx, idleIdx, cFin);
	// Attack1 → Idle
	ctrl->AddTransition(attack1Idx, idleIdx, cFin);
	// Attack2 → Idle
	ctrl->AddTransition(attack2Idx, idleIdx, cFin);
	// Attack3 → Idle
	ctrl->AddTransition(attack3Idx, idleIdx, cFin);
	// Attack4 → Idle
	ctrl->AddTransition(attack4Idx, idleIdx, cFin);

	// 가드 진입 (Idle / Run / RunEnd → Guard0)
	{
		CAnimController::Condition cG{ "Guard", CAnimController::EOp::IsTrue, 0.f };
		// 눌렀을 때 즉시
		ctrl->AddTransition(idleIdx, guard0Idx, cG, 0.1f);
		ctrl->AddTransition(runIdx, guard0Idx, cG, 0.1f);
		ctrl->AddTransition(runEndIdx, guard0Idx, cG, 0.1f);
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
		ctrl->AddTransition(guard2Idx, idleIdx, cGU, 0.05f);
	}

	CAnimController::Condition cG{ "RisingScorchingSun", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(guard0Idx, guardSkill0Idx, cG, 0.1f);
	ctrl->AddTransition(guard1Idx, guardSkill0Idx, cG, 0.1f);
	ctrl->AddTransition(guardSkill0Idx, guardSkill1Idx, cFin, 0.1f);
	ctrl->AddTransition(guardSkill1Idx, guardSkill2Idx, cFin, 0.1f);
	ctrl->AddTransition(guardSkill2Idx, idleIdx, cFin, 0.1f);

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
		Safe_Release(m_pColliderCom);
		Safe_Release(m_pAnimatroCom);
	}
}
