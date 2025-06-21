#include "BossAngryFreeze.h"
#include "BossIdle.h"
#include <EnmuArm.h>
void BossAngryFreeze::Enter(CEnmuMeat* pChar)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();
	pAnimatorLeft->SetTrigger("AngryFreeze");
	pAnimatorRight->SetTrigger("AngryFreeze");

	pChar->SetState(EnmuState::ANGRYFREEZEATTACK);
	leftArm->GetTransform()->Rotate_EulerAngles(_float3(0.0f, 90.f+180.f, 0.f)); // 왼팔 회전
	rightArm->GetTransform()->Rotate_EulerAngles(_float3(0.0f, -90.f + 180.f, 0.f)); // 오른팔 회전
}

void BossAngryFreeze::Update(CEnmuMeat* pChar, _float fTimeDelta)
{
	auto gi = CGameInstance::Get_Instance();

	m_fTimeElapsed += fTimeDelta; // 시간 증가
	if (m_fTimeElapsed >= ATTACK_END_TIME) // 공격 시간 끝나면
	{
		pChar->ChangeState(new BossIdle(TEXT("BossIdle"))); // 상태를 Idle로 변경
		return;
	}

}

void BossAngryFreeze::Exit(CEnmuMeat* pChar)
{
	auto leftArm = dynamic_cast<CEnmuArm*>(pChar->GetPart(CEnmuMeat::Parts::LEFTARM));
	auto rightArm = dynamic_cast<CEnmuArm*>(pChar->GetPart(CEnmuMeat::Parts::RIGHTARM));
	if (leftArm)
	{
		leftArm->StartRotateY(0.5f, 180.f);
	}
	if (rightArm)
	{
		rightArm->StartRotateY(0.5f, 180.f);
	}

}
