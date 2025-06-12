#include "BossSwingAttack.h"
#include "BossIdle.h"

void BossSwingAttack::Enter(CEnmuMeat* pChar)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();

	pAnimatorLeft->SetTrigger("SwingAttackStart");
	pAnimatorRight->SetTrigger("SwingAttackStart");

	pChar->SetState(EnmuState::SWINGATTACK);
}

void BossSwingAttack::Update(CEnmuMeat* pChar, _float fTimeDelta)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();

	m_fTimeElapsed += fTimeDelta;

	if (!m_bAttacked && m_fTimeElapsed >= CHARGING_TIME)
	{
		m_bAttacked = true;
		pAnimatorLeft->SetTrigger("SwingAttack");
		pAnimatorRight->SetTrigger("SwingAttack");
	}

	if (m_bAttacked && m_fTimeElapsed >= ATTACK_END_TIME)
	{
		pChar->ChangeState(new BossIdle(TEXT("Idle")));
		return;
	}
}

void BossSwingAttack::Exit(CEnmuMeat* pChar)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
}
