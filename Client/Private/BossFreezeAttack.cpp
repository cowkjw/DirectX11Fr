#include "BossFreezeAttack.h"
#include "BossIdle.h"

void BossFreezeAttack::Enter(CEnmuMeat* pChar)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();

	_int iRandHand = rand() % 2; // 0: 왼손, 1: 오른손

	if (iRandHand)
	{
		pAnimatorLeft->SetTrigger("FreezeAttackStart");
	}
	else
	{
		pAnimatorRight->SetTrigger("FreezeAttackStart");
	}
	m_bAttckArm = iRandHand;
	pChar->SetState(EnmuState::FREEZEATTACK);
}

void BossFreezeAttack::Update(CEnmuMeat* pChar, _float fTimeDelta)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();
	m_fTimeElapsed += fTimeDelta;

	_vector vTargetPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);
	_vector vMyPos = pChar->GetTransform()->Get_State(STATE::POSITION);
	_vector vDir = XMVector3Normalize(vTargetPos - vMyPos);

	if (!m_bAttacked && m_fTimeElapsed < m_fFollowTime)
	{
		if (m_bAttckArm)
			leftArm->GetTransform()->RotateToDirection(vDir);
		else
			rightArm->GetTransform()->RotateToDirection(vDir);
	}
	// 2) 추적 시간이 지났고(!m_bAttacked), 이제 한 번만 공격 트리거
	else if (!m_bAttacked && m_fTimeElapsed >= m_fFollowTime)
	{
		if (m_bAttckArm)
			pAnimatorLeft->SetTrigger("FreezeAttack");
		else
			pAnimatorRight->SetTrigger("FreezeAttack");

		m_bAttacked = true;
	}

	if (m_fTimeElapsed >= ATTACK_END_TIME)
	{
		pChar->ChangeState(new BossIdle(TEXT("Idle")));
	}
}

void BossFreezeAttack::Exit(CEnmuMeat* pChar)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);

	leftArm->GetTransform()->RotateToDirection(XMVectorSet(0.f, 0.f, 1.f, 0.f)); // 기본 방향으로 회전
	rightArm->GetTransform()->RotateToDirection(XMVectorSet(0.f, 0.f, 1.f, 0.f)); // 기본 방향으로 회전
}
