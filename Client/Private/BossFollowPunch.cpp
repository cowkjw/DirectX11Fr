#include "BossFollowPunch.h"
#include "BossIdle.h"
#include <EnmuArm.h>

void BossFollowPunch::Enter(CEnmuMeat* pChar)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();



	pAnimatorRight->SetTrigger("FollowPunchStart");

	auto pModel = static_cast<CModel*>(leftArm->Get_Component(TEXT("Com_Model")));

	if (pModel)
	{
		m_pLeftArmBone = pModel->Get_Bone("L_TentaclesArmA_7");
	}

	pModel = static_cast<CModel*>(rightArm->Get_Component(TEXT("Com_Model")));


	if (pModel)
	{
		m_pRightArmBone = pModel->Get_Bone("R_TentaclesArmA_7");
	}
	
	
	pChar->SetState(EnmuState::FOLLOWPUNCH);

	m_pWarningZone = CGameInstance::Get_Instance()->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_WarningZone"), ToIndex(LEVEL::ENMU_BOSS)
		, TEXT("WarningZone"));

	if (m_pWarningZone)
	{
		m_pWarningZone->GetTransform()->Scaling(_float3(40.f, 40.f, 40.f));
	}
}

void BossFollowPunch::Update(CEnmuMeat* pChar, _float fTimeDelta)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();
	m_fTimeElapsed += fTimeDelta;

	_vector vTargetPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);
	_vector vMyPos = pChar->GetTransform()->Get_State(STATE::POSITION);
	_vector rawDir = XMVector3Normalize(vTargetPos - vMyPos);
	// 180° 회전 보정
	_vector vDir = XMVectorSetX(rawDir, -XMVectorGetX(rawDir));
	// 아직 오른쪽 공격 안했고 공격 시간까지 안됐으면 일단 따라다니게
	if (!m_bAttackedRight && m_fTimeElapsed < RIGHTARM_START_TIME)
	{
			rightArm->GetTransform()->RotateToDirection(vDir);
		
	}
	else if (!m_bAttackedRight && m_fTimeElapsed >= RIGHTARM_START_TIME)
	{
		pAnimatorRight->SetTrigger("FollowPunch");
		pAnimatorLeft->SetTrigger("FollowPunchStart");// 왼쪽도 이제 시작하게
		m_bAttackedRight = true;
		m_fTimeElapsed = 0.f; // 오른쪽 공격 후 시간 초기화
	}
	
	if (m_bAttackedRight&&!m_bAttackedLeft && m_fTimeElapsed < LEFTARM_START_TIME)
	{
		leftArm->GetTransform()->RotateToDirection(vDir);

	}
	else if (!m_bAttackedLeft && m_fTimeElapsed >= LEFTARM_START_TIME)	
	{
	
		pAnimatorLeft->SetTrigger("FollowPunch");// 왼쪽도 이제 시작하게
		m_bAttackedLeft = true;
	}

	if (m_fTimeElapsed >= ATTACK_END_TIME)
	{
		pChar->ChangeState(new BossIdle(TEXT("Idle")));
	}
}

void BossFollowPunch::Exit(CEnmuMeat* pChar)
{
	auto leftArm = dynamic_cast<CEnmuArm*>(pChar->GetPart(CEnmuMeat::Parts::LEFTARM));
	auto rightArm = dynamic_cast<CEnmuArm*>(pChar->GetPart(CEnmuMeat::Parts::RIGHTARM));

	leftArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, 180.f, 0.f)); // 기본 방향으로 회전
	rightArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, 180.f, 0.f)); // 기본 방향으로 회전

}
