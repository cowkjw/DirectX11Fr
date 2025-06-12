#include "BossPunch.h"
#include "BossIdle.h"

void BossPunch::Enter(CEnmuMeat* pChar)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();
	leftArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, 45.f, 0.f));
	rightArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, -45.f, 0.f));
	pAnimatorLeft->SetTrigger("PunchStart");
	pAnimatorRight->SetTrigger("PunchStart");

	pChar->SetState(EnmuState::PUNCH);
}

void BossPunch::Update(CEnmuMeat* pChar, _float fTimeDelta)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();


	m_fTimeElapsed += fTimeDelta;

	if (m_fTimeElapsed >= LEFTARM_START_TIME &&!m_bAttackedLeft)
	{

		m_bAttackedLeft = true;
		pAnimatorLeft->SetTrigger("Punch");
	}

	if (m_fTimeElapsed >= RIGHTARM_START_TIME && !m_bAttackedRight)
	{
		
		m_bAttackedRight = true;
		pAnimatorRight->SetTrigger("Punch");
	}

	if (m_fTimeElapsed >= ATTACK_END_TIME)
	{
		pChar->ChangeState(new BossIdle(TEXT("Idle")));
	}
}

void BossPunch::Exit(CEnmuMeat* pChar)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	leftArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, -45.f, 0.f));
	rightArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, 45.f, 0.f));

}
