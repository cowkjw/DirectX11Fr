#include "EnmuArm.h"
#include "GameInstance.h"
#include "BodyColliderParts.h"
#include "Bone.h"
#include "Model.h"
#include <JsonLoader.h>
#include <BaseCharacter.h>
#include <ThirdPersonCamera.h>



using AniCon = CAnimController::Condition;

CEnmuArm::CEnmuArm(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnmuParts(pDevice, pContext)
{

}
CEnmuArm::CEnmuArm(const CEnmuArm& Prototype)
	: CEnmuParts(Prototype)
{
}

HRESULT CEnmuArm::Initialize(void* pArg)
{

	ARM_DESC* pArmDesc = static_cast<ARM_DESC*>(pArg);
	m_strModelKey = pArmDesc->sModelKey;

	GAMEOBJECT_DESC			Desc{};

	Desc.fRotationPerSec = XMConvertToRadians(180.f);
	Desc.fSpeedPerSec = 20.f;
	Desc.strName = m_strModelKey.find(L"Left") != _wstring::npos ? TEXT("EnmuLeftArm") : TEXT("EnmuRightArm");

	if (m_strModelKey.find(L"Left") != _wstring::npos)
	{
		m_bIsLeftArm = true; // 왼팔
	}
	else
	{
		m_bIsLeftArm = false; // 오른팔
	}

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;


	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (m_bIsLeftArm)
	{	// -21,0.0,0.0
	//	_float4 vPos = { -20.f,0.f,0.f,1.f };
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(70.f, -10.f, 15.f, 1.f));

	}
	else
	{
		// 21.68,0.0,0.0
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(-70.f, -10.f, 15.f, 1.f));
	}


	return S_OK;
}

void CEnmuArm::Priority_Update(_float fTimeDelta)
{
	for (auto& child : m_vecChildren)
	{
		child->Priority_Update(fTimeDelta);
	}
}

void CEnmuArm::Update(_float fTimeDelta)
{
	CEnmuParts::Update(fTimeDelta);
	for (auto& child : m_vecChildren)
	{
		child->Update(fTimeDelta);
	}
	if (m_bRotating)
	{
		// 1) 타이머 증가
		m_fRotateTimer += fTimeDelta;
		_float t = m_fRotateTimer / m_fRotateDur;
		if (t >= 1.f)
		{
			t = 1.f;
			m_bRotating = false;
		}

		// 2) LERP 로 Y각 계산
		_float newYaw = m_fStartYaw + (m_fTargetYaw - m_fStartYaw) * t;

		// 3) 현재 X,Z 각은 그대로, Y만 교체
		_float3 vRot = m_pTransformCom->Get_EulerAngles();
		vRot.y = newYaw;
		m_pTransformCom->Rotate_EulerAngles(vRot);

		// 보간 중에는 나머지 로직 스킵
		return;
	}
}

void CEnmuArm::Late_Update(_float fTimeDelta)
{
	CEnmuParts::Late_Update(fTimeDelta);
	for (auto& child : m_vecChildren)
	{
		child->Late_Update(fTimeDelta);
	}
}

HRESULT CEnmuArm::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CEnmuArm::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
		return E_FAIL;
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), m_strModelKey,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;


	/* For.Com_AnimController*/
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Animator"),
		TEXT("Com_Animator"), reinterpret_cast<CComponent**>(&m_pAnimatorCom), m_pModelCom)))
		return E_FAIL;


	// 애니메이션 이벤트 등록
	m_pAnimatorCom->RegisterEventListener("ActiveHitbox", [&](const string&) {
		this->ActiveCollider();
		});

	m_pAnimatorCom->RegisterEventListener("DeactiveHitbox", [&](const string&) {
		this->DeactiveCollider();
		});

	m_pAnimatorCom->RegisterEventListener("StrecthRadius", [&](const string&) {
		SetCollisionRadius(35.f);
		});

	m_pAnimatorCom->RegisterEventListener("ResetRadius", [&](const string&) {
		SetCollisionRadius(m_fDefaultRadius);
		});


	m_pAnimatorCom->RegisterEventListener("CameraShake", [&](const string& eventName) {
		if (auto pCamera = dynamic_cast<CThirdPersonCamera*>(CGameInstance::Get_Instance()->Find_GameObjectByName(ToIndex(LEVEL::ENMU_BOSS), TEXT("ThirdPersonCamera"))))
		{
			pCamera->TriggerShake(0.6f, 2.5f);
		}
		});


	if (m_bIsLeftArm)
		Ready_AnimationForLeft();
	else
		Ready_AnimationForRight();
	Ready_Collider();
	return S_OK;
}

void CEnmuArm::Ready_AnimationForRight()
{
	m_pAnimatorCom->PlayClip(m_pModelCom->GetAnimationClipByName("A_P1011_V18_C00_BaseNut01_A"));
	auto animations = m_pModelCom->GetAnimations();
	CJsonLoader jsonLoader;
	jsonLoader.LoadAnimEvent("../Asset/Json/EnmuRightArm_events.json", animations);
	jsonLoader.Free();

	auto ctrl = m_pAnimatorCom->GetAnimController();

	// Idle
	auto idleAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V18_C00_BaseNut01_A");
	idleAnim->SetLoop(true);
	size_t idleIdx = ctrl->AddState("Idle", idleAnim, m_pModelCom->GetAnimationMap()[idleAnim->Get_Name()]);

	// 일자 펀치 (0~1은 기모으기 2에서 펀치 3~4가 돌아오는)
	vector<CAnimation*> punchAnims;
	for (int i = 0; i < 5; ++i)
	{
		auto punchAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V18_C00_AtkSkl01_" + to_string(i));
		punchAnim->SetLoop(false);

		punchAnims.push_back(punchAnim);
	}
	size_t punchIdx = ctrl->AddState("Punch0", punchAnims[0], m_pModelCom->GetAnimationMap()[punchAnims[0]->Get_Name()]);
	size_t punchIdx1 = ctrl->AddState("Punch1", punchAnims[1], m_pModelCom->GetAnimationMap()[punchAnims[1]->Get_Name()]);
	size_t punchIdx2 = ctrl->AddState("Punch2", punchAnims[2], m_pModelCom->GetAnimationMap()[punchAnims[2]->Get_Name()]);
	size_t punchIdx3 = ctrl->AddState("Punch3", punchAnims[3], m_pModelCom->GetAnimationMap()[punchAnims[3]->Get_Name()]);
	size_t punchIdx4 = ctrl->AddState("Punch4", punchAnims[4], m_pModelCom->GetAnimationMap()[punchAnims[4]->Get_Name()]);


	// 플레이어 위치에 펀치 (0~1 기모으기 2에서 내려찍고 3~4가 돌아오는)
	vector<CAnimation*> followPunchAnims;
	for (int i = 0; i < 5; ++i)
	{
		auto punchAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V18_C00_AtkSkl04_" + to_string(i));
		punchAnim->SetLoop(false);
		followPunchAnims.push_back(punchAnim);
	}

	size_t followPunchIdx = ctrl->AddState("FollowPunch0", followPunchAnims[0], m_pModelCom->GetAnimationMap()[followPunchAnims[0]->Get_Name()]);
	size_t followPunchIdx1 = ctrl->AddState("FollowPunch1", followPunchAnims[1], m_pModelCom->GetAnimationMap()[followPunchAnims[1]->Get_Name()]);
	size_t followPunchIdx2 = ctrl->AddState("FollowPunch2", followPunchAnims[2], m_pModelCom->GetAnimationMap()[followPunchAnims[2]->Get_Name()]);
	size_t followPunchIdx3 = ctrl->AddState("FollowPunch3", followPunchAnims[3], m_pModelCom->GetAnimationMap()[followPunchAnims[3]->Get_Name()]);
	size_t followPunchIdx4 = ctrl->AddState("FollowPunch4", followPunchAnims[4], m_pModelCom->GetAnimationMap()[followPunchAnims[4]->Get_Name()]);

	// 손바닥으로 치기 (0~1 기모으기 2에서 내려찍고 3~4가 돌아오는)
	vector<CAnimation*> handAnims;
	for (int i = 0; i < 5; ++i)
	{
		auto handAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V18_C00_AtkSkl03_" + to_string(i));
		handAnim->SetLoop(false);
		handAnims.push_back(handAnim);
	}

	size_t handIdx = ctrl->AddState("HandAttack0", handAnims[0], m_pModelCom->GetAnimationMap()[handAnims[0]->Get_Name()]);
	size_t handIdx1 = ctrl->AddState("HandAttack1", handAnims[1], m_pModelCom->GetAnimationMap()[handAnims[1]->Get_Name()]);
	size_t handIdx2 = ctrl->AddState("HandAttack2", handAnims[2], m_pModelCom->GetAnimationMap()[handAnims[2]->Get_Name()]);
	size_t handIdx3 = ctrl->AddState("HandAttack3", handAnims[3], m_pModelCom->GetAnimationMap()[handAnims[3]->Get_Name()]);
	size_t handIdx4 = ctrl->AddState("HandAttack4", handAnims[4], m_pModelCom->GetAnimationMap()[handAnims[4]->Get_Name()]);

	// 촉수 패턴 (0이 준비 1이 내려찍고 Loop 2가 올라오는)
	vector<CAnimation*> tentacleAnims;
	for (int i = 0; i < 3; ++i)
	{
		auto tentacleAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V18_C00_AtkSkl05_" + to_string(i));
		tentacleAnim->SetLoop(false);
		tentacleAnims.push_back(tentacleAnim);
	}

	size_t tentacleIdx = ctrl->AddState("TentacleAttack0", tentacleAnims[0], m_pModelCom->GetAnimationMap()[tentacleAnims[0]->Get_Name()]);
	size_t tentacleIdx1 = ctrl->AddState("TentacleAttack1", tentacleAnims[1], m_pModelCom->GetAnimationMap()[tentacleAnims[1]->Get_Name()]);
	size_t tentacleIdx2 = ctrl->AddState("TentacleAttack2", tentacleAnims[2], m_pModelCom->GetAnimationMap()[tentacleAnims[2]->Get_Name()]);


	// 원 영역이 따라다니면서 일정 시간 지나서 플레이어가 맞으면 입력으로 풀게 하는 패턴
	// 0이 준비 1이 준비후 잠깐 보여주는 애니메이션 2가 발동 3이 발동하고 잠깐 4가 돌아가는
	vector<CAnimation*> freezeAnims;

	for (int i = 0; i < 5; ++i)
	{
		auto freezeAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V18_C00_AtkSkl07_" + to_string(i));
		freezeAnim->SetLoop(false);
		freezeAnims.push_back(freezeAnim);
	}
	size_t freezeIdx = ctrl->AddState("FreezeAttack0", freezeAnims[0], m_pModelCom->GetAnimationMap()[freezeAnims[0]->Get_Name()]);
	size_t freezeIdx1 = ctrl->AddState("FreezeAttack1", freezeAnims[1], m_pModelCom->GetAnimationMap()[freezeAnims[1]->Get_Name()]);
	size_t freezeIdx2 = ctrl->AddState("FreezeAttack2", freezeAnims[2], m_pModelCom->GetAnimationMap()[freezeAnims[2]->Get_Name()]);
	size_t freezeIdx3 = ctrl->AddState("FreezeAttack3", freezeAnims[3], m_pModelCom->GetAnimationMap()[freezeAnims[3]->Get_Name()]);
	size_t freezeIdx4 = ctrl->AddState("FreezeAttack4", freezeAnims[4], m_pModelCom->GetAnimationMap()[freezeAnims[4]->Get_Name()]);

	auto angryFreezeAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V18_C00_AtkSkl08");
	angryFreezeAnim->SetLoop(true);
	size_t angryFreezeIdx = ctrl->AddState("AngryFreeze", angryFreezeAnim, m_pModelCom->GetAnimationMap()[angryFreezeAnim->Get_Name()]);

	// 휘두르는 패턴 (0이 준비 1이 기모으기 2가 휘두르고 돌아가기)
	vector<CAnimation*> swingAnims;
	for (int i = 0; i < 3; ++i)
	{
		auto swingAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V18_C00_AtkSkl09_" + to_string(i));
		swingAnim->SetLoop(false);
		swingAnims.push_back(swingAnim);
	}
	size_t swingIdx = ctrl->AddState("SwingAttack0", swingAnims[0], m_pModelCom->GetAnimationMap()[swingAnims[0]->Get_Name()]);
	size_t swingIdx1 = ctrl->AddState("SwingAttack1", swingAnims[1], m_pModelCom->GetAnimationMap()[swingAnims[1]->Get_Name()]);
	size_t swingIdx2 = ctrl->AddState("SwingAttack2", swingAnims[2], m_pModelCom->GetAnimationMap()[swingAnims[2]->Get_Name()]);


	vector<CAnimation*> openAnims;
	for (int i = 0; i < 3; i++)
	{
		auto openAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V18_C00_DmgDown01_" + to_string(i));
		openAnim->SetLoop(false);
		openAnims.push_back(openAnim);
	}

	size_t openIdx = ctrl->AddState("Open0", openAnims[0], m_pModelCom->GetAnimationMap()[openAnims[0]->Get_Name()]);
	size_t openIdx1 = ctrl->AddState("Open1", openAnims[1], m_pModelCom->GetAnimationMap()[openAnims[1]->Get_Name()]);
	size_t openIdx2 = ctrl->AddState("Open1", openAnims[2], m_pModelCom->GetAnimationMap()[openAnims[2]->Get_Name()]);



	// 기모으는 단계는 일단 루프로
	punchAnims[1]->SetLoop(true); // 0번은 루프
	followPunchAnims[1]->SetLoop(true); // 0번은 루프
	handAnims[1]->SetLoop(true); // 0번은 루프
	tentacleAnims[1]->SetLoop(true); // 0번은 루프
	freezeAnims[1]->SetLoop(true); // 0번은 루프
	swingAnims[1]->SetLoop(true); // 0번은 루프
	openAnims[1]->SetLoop(true); // 0번은 루프



	// 상태 등록 (준비 단계)
	m_pAnimatorCom->AddTrigger("PunchStart");
	m_pAnimatorCom->AddTrigger("FollowPunchStart");
	m_pAnimatorCom->AddTrigger("HandAttackStart");
	m_pAnimatorCom->AddTrigger("TentacleAttackStart");
	m_pAnimatorCom->AddTrigger("FreezeAttackStart");
	m_pAnimatorCom->AddTrigger("SwingAttackStart");
	m_pAnimatorCom->AddTrigger("OpenStart");
	m_pAnimatorCom->AddTrigger("OpenEnd");

	// 상태 등록 (실행 단계)
	m_pAnimatorCom->AddTrigger("Punch");
	m_pAnimatorCom->AddTrigger("FollowPunch");
	m_pAnimatorCom->AddTrigger("HandAttack");
	m_pAnimatorCom->AddTrigger("FreezeAttack");
	m_pAnimatorCom->AddTrigger("AngryFreeze");
	m_pAnimatorCom->AddTrigger("SwingAttack");
	m_pAnimatorCom->AddBool("Open");
	m_pAnimatorCom->AddBool("TentacleAttackEnd");


	// 기본으로 끝나고 애니메이션 처리
	AniCon cFinished{ "", CAnimController::EOp::Finished, 0.f };

	// 상태 전환
	AniCon cPunchStart{ "PunchStart", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, punchIdx, cPunchStart, 0.1f);
	ctrl->AddTransition(punchIdx, punchIdx1, cFinished, 0.1f); // 기모으기

	AniCon cPunch{ "Punch", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(punchIdx1, punchIdx2, cPunch, 0.1f); // 펀치
	ctrl->AddTransition(punchIdx2, punchIdx3, cFinished, 0.1f); // 펀치 후 돌아오기
	ctrl->AddTransition(punchIdx3, punchIdx4, cFinished, 0.1f); // 펀치 후 돌아오기
	ctrl->AddTransition(punchIdx4, idleIdx, cFinished, 0.1f); // 펀치 후 돌아오기

	AniCon cFollowPunchStart{ "FollowPunchStart", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, followPunchIdx, cFollowPunchStart, 0.1f);
	ctrl->AddTransition(followPunchIdx, followPunchIdx1, cFinished, 0.1f); // 기모으기
	AniCon cFollowPunch{ "FollowPunch", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(followPunchIdx1, followPunchIdx2, cFollowPunch, 0.1f); // 펀치
	ctrl->AddTransition(followPunchIdx2, followPunchIdx3, cFinished, 0.1f); // 펀치 후 돌아오기
	ctrl->AddTransition(followPunchIdx3, followPunchIdx4, cFinished, 0.1f); // 펀치 후 돌아오기
	ctrl->AddTransition(followPunchIdx4, idleIdx, cFinished, 0.1f); // 펀치 후 돌아오기

	AniCon cHandAttackStart{ "HandAttackStart", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, handIdx, cHandAttackStart, 0.1f);
	ctrl->AddTransition(handIdx, handIdx1, cFinished, 0.1f); // 기모으기

	AniCon cHandAttack{ "HandAttack", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(handIdx1, handIdx2, cHandAttack, 0.1f); // 손바닥 공격
	ctrl->AddTransition(handIdx2, handIdx3, cFinished, 0.1f); // 손바닥 공격 후 돌아오기
	ctrl->AddTransition(handIdx3, handIdx4, cFinished, 0.1f); // 손바닥 공격 후 돌아오기
	ctrl->AddTransition(handIdx4, idleIdx, cFinished, 0.1f); // 손바닥 공격 후 돌아오기

	AniCon cTentacleAttackStart{ "TentacleAttackStart", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, tentacleIdx, cTentacleAttackStart, 0.1f);
	ctrl->AddTransition(tentacleIdx, tentacleIdx1, cFinished, 0.1f); // 기모으기
	AniCon cTentacleAttackEnd{ "TentacleAttackEnd", CAnimController::EOp::IsTrue, 0.f };
	ctrl->AddTransition(tentacleIdx1, tentacleIdx2, cTentacleAttackEnd, 0.1f); // 촉수 공격
	ctrl->AddTransition(tentacleIdx2, idleIdx, cFinished, 0.1f); // 촉수 공격 후 돌아오기

	AniCon cFreezeAttackStart{ "FreezeAttackStart", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, freezeIdx, cFreezeAttackStart, 0.1f);
	ctrl->AddTransition(freezeIdx, freezeIdx1, cFinished, 0.1f); // 기모으기
	AniCon cFreezeAttack{ "FreezeAttack", CAnimController::EOp::Trigger, 0.f };

	ctrl->AddTransition(freezeIdx1, freezeIdx2, cFreezeAttack, 0.1f); // 얼리는 공격
	ctrl->AddTransition(freezeIdx2, freezeIdx3, cFinished, 0.1f); // 얼리는 공격 후 돌아오기
	ctrl->AddTransition(freezeIdx3, freezeIdx4, cFinished, 0.1f); // 얼리는 공격 후 돌아오기
	ctrl->AddTransition(freezeIdx4, idleIdx, cFinished, 0.1f); // 얼리는 공격 후 돌아오기

	AniCon cAngryFreezeStart{ "AngryFreeze", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, angryFreezeIdx, cAngryFreezeStart, 0.1f);
	ctrl->AddTransition(angryFreezeIdx, idleIdx, cFinished, 0.1f); // 화난 얼리기 공격 후 돌아오기

	AniCon cSwingAttackStart{ "SwingAttackStart", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, swingIdx, cSwingAttackStart, 0.1f);
	ctrl->AddTransition(swingIdx, swingIdx1, cFinished, 0.1f); // 기모으기

	AniCon cSwingAttack{ "SwingAttack", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(swingIdx1, swingIdx2, cSwingAttack, 0.1f); // 휘두르기
	ctrl->AddTransition(swingIdx2, idleIdx, cFinished, 0.1f); // 휘두르기 후 돌아오기


	// 상태 전환
	AniCon cOpenStart{ "OpenStart", CAnimController::EOp::Trigger, 0.f };

	ctrl->AddTransition(idleIdx, openIdx, cOpenStart, 0.1f);
	ctrl->AddTransition(openIdx, openIdx1, cFinished, 0.1f); // 열리는거

	AniCon cOpen{ "Open", CAnimController::EOp::IsFalse, 0.f };
	ctrl->AddTransition(openIdx1, openIdx2, cOpen, 0.1f); // 열리는거
	ctrl->AddTransition(openIdx2, idleIdx, cFinished, 0.1f); // 열리는거


}
void CEnmuArm::Ready_AnimationForLeft()
{
	m_pAnimatorCom->PlayClip(m_pModelCom->GetAnimationClipByName("A_P1011_V19_C00_BaseNut01_A"));
	auto animations = m_pModelCom->GetAnimations();
	CJsonLoader jsonLoader;
	jsonLoader.LoadAnimEvent("../Asset/Json/EnmuLeftArm_events.json", animations);
	jsonLoader.Free();

	auto ctrl = m_pAnimatorCom->GetAnimController();

	// Idle
	auto idleAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V19_C00_BaseNut01_A");
	idleAnim->SetLoop(true);
	size_t idleIdx = ctrl->AddState("Idle", idleAnim, m_pModelCom->GetAnimationMap()[idleAnim->Get_Name()]);

	// 일자 펀치 (0~1은 기모으기 2에서 펀치 3~4가 돌아오는)
	vector<CAnimation*> punchAnims;
	for (int i = 0; i < 5; ++i)
	{
		auto punchAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V19_C00_AtkSkl01_" + to_string(i));
		punchAnim->SetLoop(false);

		punchAnims.push_back(punchAnim);
	}
	size_t punchIdx = ctrl->AddState("Punch0", punchAnims[0], m_pModelCom->GetAnimationMap()[punchAnims[0]->Get_Name()]);
	size_t punchIdx1 = ctrl->AddState("Punch1", punchAnims[1], m_pModelCom->GetAnimationMap()[punchAnims[1]->Get_Name()]);
	size_t punchIdx2 = ctrl->AddState("Punch2", punchAnims[2], m_pModelCom->GetAnimationMap()[punchAnims[2]->Get_Name()]);
	size_t punchIdx3 = ctrl->AddState("Punch3", punchAnims[3], m_pModelCom->GetAnimationMap()[punchAnims[3]->Get_Name()]);
	size_t punchIdx4 = ctrl->AddState("Punch4", punchAnims[4], m_pModelCom->GetAnimationMap()[punchAnims[4]->Get_Name()]);


	// 플레이어 위치에 펀치 (0~1 기모으기 2에서 내려찍고 3~4가 돌아오는)
	vector<CAnimation*> followPunchAnims;
	for (int i = 0; i < 5; ++i)
	{
		auto punchAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V19_C00_AtkSkl04_" + to_string(i));
		punchAnim->SetLoop(false);
		followPunchAnims.push_back(punchAnim);
	}

	size_t followPunchIdx = ctrl->AddState("FollowPunch0", followPunchAnims[0], m_pModelCom->GetAnimationMap()[followPunchAnims[0]->Get_Name()]);
	size_t followPunchIdx1 = ctrl->AddState("FollowPunch1", followPunchAnims[1], m_pModelCom->GetAnimationMap()[followPunchAnims[1]->Get_Name()]);
	size_t followPunchIdx2 = ctrl->AddState("FollowPunch2", followPunchAnims[2], m_pModelCom->GetAnimationMap()[followPunchAnims[2]->Get_Name()]);
	size_t followPunchIdx3 = ctrl->AddState("FollowPunch3", followPunchAnims[3], m_pModelCom->GetAnimationMap()[followPunchAnims[3]->Get_Name()]);
	size_t followPunchIdx4 = ctrl->AddState("FollowPunch4", followPunchAnims[4], m_pModelCom->GetAnimationMap()[followPunchAnims[4]->Get_Name()]);

	// 손바닥으로 치기 (0~1 기모으기 2에서 내려찍고 3~4가 돌아오는)
	vector<CAnimation*> handAnims;
	for (int i = 0; i < 5; ++i)
	{
		auto handAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V19_C00_AtkSkl03_" + to_string(i));
		handAnim->SetLoop(false);
		handAnims.push_back(handAnim);
	}

	size_t handIdx = ctrl->AddState("HandAttack0", handAnims[0], m_pModelCom->GetAnimationMap()[handAnims[0]->Get_Name()]);
	size_t handIdx1 = ctrl->AddState("HandAttack1", handAnims[1], m_pModelCom->GetAnimationMap()[handAnims[1]->Get_Name()]);
	size_t handIdx2 = ctrl->AddState("HandAttack2", handAnims[2], m_pModelCom->GetAnimationMap()[handAnims[2]->Get_Name()]);
	size_t handIdx3 = ctrl->AddState("HandAttack3", handAnims[3], m_pModelCom->GetAnimationMap()[handAnims[3]->Get_Name()]);
	size_t handIdx4 = ctrl->AddState("HandAttack4", handAnims[4], m_pModelCom->GetAnimationMap()[handAnims[4]->Get_Name()]);

	// 촉수 패턴 (0이 준비 1이 내려찍고 Loop 2가 올라오는)
	vector<CAnimation*> tentacleAnims;
	for (int i = 0; i < 3; ++i)
	{
		auto tentacleAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V19_C00_AtkSkl05_" + to_string(i));
		tentacleAnim->SetLoop(false);
		tentacleAnims.push_back(tentacleAnim);
	}

	size_t tentacleIdx = ctrl->AddState("TentacleAttack0", tentacleAnims[0], m_pModelCom->GetAnimationMap()[tentacleAnims[0]->Get_Name()]);
	size_t tentacleIdx1 = ctrl->AddState("TentacleAttack1", tentacleAnims[1], m_pModelCom->GetAnimationMap()[tentacleAnims[1]->Get_Name()]);
	size_t tentacleIdx2 = ctrl->AddState("TentacleAttack2", tentacleAnims[2], m_pModelCom->GetAnimationMap()[tentacleAnims[2]->Get_Name()]);


	// 원 영역이 따라다니면서 일정 시간 지나서 플레이어가 맞으면 입력으로 풀게 하는 패턴
	// 0이 준비 1이 준비후 잠깐 보여주는 애니메이션 2가 발동 3이 발동하고 잠깐 4가 돌아가는
	vector<CAnimation*> freezeAnims;

	for (int i = 0; i < 5; ++i)
	{
		auto freezeAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V19_C00_AtkSkl07_" + to_string(i));
		freezeAnim->SetLoop(false);
		freezeAnims.push_back(freezeAnim);
	}
	size_t freezeIdx = ctrl->AddState("FreezeAttack0", freezeAnims[0], m_pModelCom->GetAnimationMap()[freezeAnims[0]->Get_Name()]);
	size_t freezeIdx1 = ctrl->AddState("FreezeAttack1", freezeAnims[1], m_pModelCom->GetAnimationMap()[freezeAnims[1]->Get_Name()]);
	size_t freezeIdx2 = ctrl->AddState("FreezeAttack2", freezeAnims[2], m_pModelCom->GetAnimationMap()[freezeAnims[2]->Get_Name()]);
	size_t freezeIdx3 = ctrl->AddState("FreezeAttack3", freezeAnims[3], m_pModelCom->GetAnimationMap()[freezeAnims[3]->Get_Name()]);
	size_t freezeIdx4 = ctrl->AddState("FreezeAttack4", freezeAnims[4], m_pModelCom->GetAnimationMap()[freezeAnims[4]->Get_Name()]);

	auto angryFreezeAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V19_C00_AtkSkl08");
	angryFreezeAnim->SetLoop(true);
	size_t angryFreezeIdx = ctrl->AddState("AngryFreeze", angryFreezeAnim, m_pModelCom->GetAnimationMap()[angryFreezeAnim->Get_Name()]);

	// 휘두르는 패턴 (0이 준비 1이 기모으기 2가 휘두르고 돌아가기)
	vector<CAnimation*> swingAnims;
	for (int i = 0; i < 3; ++i)
	{
		auto swingAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V19_C00_AtkSkl09_" + to_string(i));
		swingAnim->SetLoop(false);
		swingAnims.push_back(swingAnim);
	}
	size_t swingIdx = ctrl->AddState("SwingAttack0", swingAnims[0], m_pModelCom->GetAnimationMap()[swingAnims[0]->Get_Name()]);
	size_t swingIdx1 = ctrl->AddState("SwingAttack1", swingAnims[1], m_pModelCom->GetAnimationMap()[swingAnims[1]->Get_Name()]);
	size_t swingIdx2 = ctrl->AddState("SwingAttack2", swingAnims[2], m_pModelCom->GetAnimationMap()[swingAnims[2]->Get_Name()]);

	vector<CAnimation*> openAnims;
	for (int i = 0; i < 3; i++)
	{
		auto openAnim = m_pModelCom->GetAnimationClipByName("A_P1011_V19_C00_DmgDown01_" + to_string(i));
		openAnim->SetLoop(false);
		openAnims.push_back(openAnim);
	}

	size_t openIdx = ctrl->AddState("Open0", openAnims[0], m_pModelCom->GetAnimationMap()[openAnims[0]->Get_Name()]);
	size_t openIdx1 = ctrl->AddState("Open1", openAnims[1], m_pModelCom->GetAnimationMap()[openAnims[1]->Get_Name()]);
	size_t openIdx2 = ctrl->AddState("Open1", openAnims[2], m_pModelCom->GetAnimationMap()[openAnims[2]->Get_Name()]);



	// 기모으는 단계는 일단 루프로
	openAnims[1]->SetLoop(true); // 0번은 루프
	punchAnims[1]->SetLoop(true); // 0번은 루프
	followPunchAnims[1]->SetLoop(true); // 0번은 루프
	handAnims[1]->SetLoop(true); // 0번은 루프
	tentacleAnims[1]->SetLoop(true); // 0번은 루프
	freezeAnims[1]->SetLoop(true); // 0번은 루프
	swingAnims[1]->SetLoop(true); // 0번은 루프




	// 상태 등록 (준비 단계)
	m_pAnimatorCom->AddTrigger("PunchStart");
	m_pAnimatorCom->AddTrigger("FollowPunchStart");
	m_pAnimatorCom->AddTrigger("HandAttackStart");
	m_pAnimatorCom->AddTrigger("TentacleAttackStart");
	m_pAnimatorCom->AddTrigger("FreezeAttackStart");
	m_pAnimatorCom->AddTrigger("SwingAttackStart");
	m_pAnimatorCom->AddTrigger("OpenStart");
	m_pAnimatorCom->AddTrigger("OpenEnd");

	// 상태 등록 (실행 단계)
	m_pAnimatorCom->AddTrigger("Punch");
	m_pAnimatorCom->AddTrigger("FollowPunch");
	m_pAnimatorCom->AddTrigger("HandAttack");
	m_pAnimatorCom->AddTrigger("FreezeAttack");
	m_pAnimatorCom->AddTrigger("AngryFreeze");
	m_pAnimatorCom->AddTrigger("SwingAttack");
	m_pAnimatorCom->AddBool("Open");
	m_pAnimatorCom->AddBool("TentacleAttackEnd");

	// 기본으로 끝나고 애니메이션 처리
	AniCon cFinished{ "", CAnimController::EOp::Finished, 0.f };

	// 상태 전환
	AniCon cPunchStart{ "PunchStart", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, punchIdx, cPunchStart, 0.1f);
	ctrl->AddTransition(punchIdx, punchIdx1, cFinished, 0.1f); // 기모으기

	AniCon cPunch{ "Punch", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(punchIdx1, punchIdx2, cPunch, 0.1f); // 펀치
	ctrl->AddTransition(punchIdx2, punchIdx3, cFinished, 0.1f); // 펀치 후 돌아오기
	ctrl->AddTransition(punchIdx3, punchIdx4, cFinished, 0.1f); // 펀치 후 돌아오기
	ctrl->AddTransition(punchIdx4, idleIdx, cFinished, 0.1f); // 펀치 후 돌아오기

	AniCon cFollowPunchStart{ "FollowPunchStart", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, followPunchIdx, cFollowPunchStart, 0.1f);
	ctrl->AddTransition(followPunchIdx, followPunchIdx1, cFinished, 0.1f); // 기모으기
	AniCon cFollowPunch{ "FollowPunch", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(followPunchIdx1, followPunchIdx2, cFollowPunch, 0.1f); // 펀치
	ctrl->AddTransition(followPunchIdx2, followPunchIdx3, cFinished, 0.1f); // 펀치 후 돌아오기
	ctrl->AddTransition(followPunchIdx3, followPunchIdx4, cFinished, 0.1f); // 펀치 후 돌아오기
	ctrl->AddTransition(followPunchIdx4, idleIdx, cFinished, 0.1f); // 펀치 후 돌아오기

	AniCon cHandAttackStart{ "HandAttackStart", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, handIdx, cHandAttackStart, 0.1f);
	ctrl->AddTransition(handIdx, handIdx1, cFinished, 0.1f); // 기모으기

	AniCon cHandAttack{ "HandAttack", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(handIdx1, handIdx2, cHandAttack, 0.1f); // 손바닥 공격
	ctrl->AddTransition(handIdx2, handIdx3, cFinished, 0.1f); // 손바닥 공격 후 돌아오기
	ctrl->AddTransition(handIdx3, handIdx4, cFinished, 0.1f); // 손바닥 공격 후 돌아오기
	ctrl->AddTransition(handIdx4, idleIdx, cFinished, 0.1f); // 손바닥 공격 후 돌아오기

	AniCon cTentacleAttackStart{ "TentacleAttackStart", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, tentacleIdx, cTentacleAttackStart, 0.1f);
	ctrl->AddTransition(tentacleIdx, tentacleIdx1, cFinished, 0.1f); // 기모으기
	AniCon cTentacleAttackEnd{ "TentacleAttackEnd", CAnimController::EOp::IsTrue, 0.f };
	ctrl->AddTransition(tentacleIdx1, tentacleIdx2, cTentacleAttackEnd, 0.1f); // 촉수 공격
	ctrl->AddTransition(tentacleIdx2, idleIdx, cFinished, 0.1f); // 촉수 공격 후 돌아오기

	AniCon cFreezAttackStart{ "FreezeAttackStart", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, freezeIdx, cFreezAttackStart, 0.1f);
	ctrl->AddTransition(freezeIdx, freezeIdx1, cFinished, 0.1f); // 기모으기
	AniCon cFreezAttack{ "FreezeAttack", CAnimController::EOp::Trigger, 0.f };

	ctrl->AddTransition(freezeIdx1, freezeIdx2, cFreezAttack, 0.1f); // 얼리는 공격
	ctrl->AddTransition(freezeIdx2, freezeIdx3, cFinished, 0.1f); // 얼리는 공격 후 돌아오기
	ctrl->AddTransition(freezeIdx3, freezeIdx4, cFinished, 0.1f); // 얼리는 공격 후 돌아오기
	ctrl->AddTransition(freezeIdx4, idleIdx, cFinished, 0.1f); // 얼리는 공격 후 돌아오기

	AniCon cAngryFreezStart{ "AngryFreeze", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, angryFreezeIdx, cAngryFreezStart, 0.1f);
	ctrl->AddTransition(angryFreezeIdx, idleIdx, cFinished, 0.1f); // 화난 얼리기 공격 후 돌아오기

	AniCon cSwingAttackStart{ "SwingAttackStart", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(idleIdx, swingIdx, cSwingAttackStart, 0.1f);
	ctrl->AddTransition(swingIdx, swingIdx1, cFinished, 0.1f); // 기모으기

	AniCon cSwingAttack{ "SwingAttack", CAnimController::EOp::Trigger, 0.f };
	ctrl->AddTransition(swingIdx1, swingIdx2, cSwingAttack, 0.1f); // 휘두르기
	ctrl->AddTransition(swingIdx2, idleIdx, cFinished, 0.1f); // 휘두르기 후 돌아오기

	// 상태 전환
	AniCon cOpenStart{ "OpenStart", CAnimController::EOp::Trigger, 0.f };

	ctrl->AddTransition(idleIdx, openIdx, cOpenStart, 0.1f);
	ctrl->AddTransition(openIdx, openIdx1, cFinished, 0.1f); // 열리는거

	AniCon cOpen{ "Open", CAnimController::EOp::IsFalse, 0.f };
	ctrl->AddTransition(openIdx1, openIdx2, cOpen, 0.1f); // 열리는거
	ctrl->AddTransition(openIdx2, idleIdx, cFinished, 0.1f); // 열리는거


}


void CEnmuArm::Ready_Collider()
{
	CBodyColliderParts::BODYCOLLIDERPARTS_DESC desc{};
	desc.vColliderOffsets.push_back(_float3(0.f, 0.f, 0.f));
	desc.fRadius = 30.f;
	if (m_bIsLeftArm)
	{
		m_pBodyColliderCom = CBodyColliderParts::Create(m_pDevice, m_pContext);
		m_pBodyColliderCom->Set_BoneSocket(m_pModelCom->Get_Bone("L_Hand"));
	}
	else
	{
		m_pBodyColliderCom = CBodyColliderParts::Create(m_pDevice, m_pContext);
		m_pBodyColliderCom->Set_BoneSocket(m_pModelCom->Get_Bone("R_Hand"));
	}
	this->AddChild(m_pBodyColliderCom);
	m_pBodyColliderCom->Initialize(&desc);
}

void CEnmuArm::ActiveCollider()
{
	if (m_pBodyColliderCom)
	{
		m_pBodyColliderCom->SetActive(true);
	}
}

void CEnmuArm::DeactiveCollider()
{
	if (m_pBodyColliderCom)
	{
		m_pBodyColliderCom->SetActive(false);
	}
}

void CEnmuArm::SetCollisionRadius(_float fRadius)
{
	m_pBodyColliderCom->Set_Radius(0,fRadius);
}

void CEnmuArm::StartRotateY(_float duration, _float targetYawDeg)
{
	auto v = m_pTransformCom->Get_EulerAngles();
	m_fStartYaw = v.y;                    // 현재 Y값 저장
	m_fTargetYaw = targetYawDeg;           // 목표 Y값 세팅
	m_fRotateDur = duration;
	m_fRotateTimer = 0.f;

	if (m_fStartYaw == m_fTargetYaw) // 현재 Y값과 목표 Y값이 같으면 회전하지 않음
		return;
	m_bRotating = true;
}

CEnmuArm* CEnmuArm::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CEnmuArm* pInstance = new CEnmuArm(pDevice, pContext);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CEnmuArm");
		Safe_Release(pInstance);
	}
	return pInstance;
}

// 딱히 클론할 필요가 없음
CGameObject* CEnmuArm::Clone(void* pArg)
{
	return nullptr;
}

void CEnmuArm::Free()
{
	CEnmuParts::Free();
	Safe_Release(m_pBodyColliderCom);
}

void CEnmuArm::OnCollisionEnter(CCollider* other)
{
}

void CEnmuArm::OnCollisionStay(CCollider* other, float fTimeDelta)
{
}

void CEnmuArm::OnCollisionExit(CCollider* other)
{
}
