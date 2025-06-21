#include "BossHandAttack.h"
#include "BossIdle.h"
#include <EnmuArm.h>

void BossHandAttack::Enter(CEnmuMeat* pChar)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();
	leftArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, 45.f+180.f, 0.f));
	rightArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, -45.f+180.f, 0.f));
	pAnimatorLeft->SetTrigger("HandAttackStart");
	pAnimatorRight->SetTrigger("HandAttackStart");

	pChar->SetState(EnmuState::HANDATTACK);

	for (_int i = 0; i < m_pWarnings.size(); i++)
	{
		auto pWarning = CGameInstance::Get_Instance()->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_WarningZone"), ToIndex(LEVEL::ENMU_BOSS)
			, TEXT("WarningZone"));
		if (pWarning)
		{
			if (i == 0)
			{
				pWarning->GetTransform()->Set_State(STATE::POSITION, XMVectorSet(15.f, -17.772f, -90.248f, 1.f));
			}
			else if (i == 1)
			{
				pWarning->GetTransform()->Set_State(STATE::POSITION, XMVectorSet(-2.230f, -17.772f, -90.248f, 1.f));
			}
			pWarning->GetTransform()->Scaling(_float3(65.f, 65.f, 65.f));
			m_pWarnings[i] = pWarning;
		}
	
	}
	if(m_pWarnings[1])
		m_pWarnings[1]->SetActive(false);
}

void BossHandAttack::Update(CEnmuMeat* pChar, _float fTimeDelta)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();


	m_fTimeElapsed += fTimeDelta;

	if (m_fTimeElapsed >= LEFTARM_START_TIME && !m_bAttackedLeft)
	{
		if (m_pWarnings[0] && m_pWarnings[1])
		{
			m_pWarnings[0]->SetActive(false);
			m_pWarnings[1]->SetActive(true);
		}
		m_bAttackedLeft = true;
		pAnimatorLeft->SetTrigger("HandAttack");
	}

	if (m_fTimeElapsed >= RIGHTARM_START_TIME && !m_bAttackedRight)
	{
		if (m_pWarnings[1])
		{
			m_pWarnings[1]->SetActive(false);
		}
		m_bAttackedRight = true;
		pAnimatorRight->SetTrigger("HandAttack");
	}

	if (m_fTimeElapsed >= ATTACK_END_TIME)
	{
		for (_int i = 0; i < m_pWarnings.size(); i++)
		{
			if (m_pWarnings[i])
			{
				m_pWarnings[i]->SetActive(false);
			}
		}
		pChar->ChangeState(new BossIdle(TEXT("Idle")));
	}
}

void BossHandAttack::Exit(CEnmuMeat* pChar)
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
