#include "BossPunch.h"
#include "BossIdle.h"
#include <EnmuArm.h>

void BossPunch::Enter(CEnmuMeat* pChar)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();
	leftArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, 45.f+180.f, 0.f));
	rightArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, -45.f+180.f, 0.f));
	pAnimatorLeft->SetTrigger("PunchStart");
	pAnimatorRight->SetTrigger("PunchStart");

	pChar->SetState(EnmuState::PUNCH);

	for (_int i = 0; i < m_pWarnings.size(); i++)
	{
		auto pWarning = dynamic_cast<CWarningZoneDecal*>(CGameInstance::Get_Instance()->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_WarningZone"), ToIndex(LEVEL::ENMU_BOSS)
			, TEXT("WarningZone")));
		if (pWarning)
		{
			pWarning->SetShaderPass(1);
			if (i == 0)
			{
				pWarning->GetTransform()->Set_State(STATE::POSITION, XMVectorSet(4.f, -17.772f, -93.248f, 1.f));
				pWarning->GetTransform()->Rotate_EulerAngles(_float3(90.f, -45.f, 0.f));
			}
			else if (i == 1)
			{
				pWarning->GetTransform()->Set_State(STATE::POSITION, XMVectorSet(0.f, -17.772f, -90.248f, 1.f));
				pWarning->GetTransform()->Rotate_EulerAngles(_float3(90.f, 45.f, 0.f));
			}
			pWarning->GetTransform()->Scaling(_float3(60.f, 300.f, 65.f));
			m_pWarnings[i] = pWarning;
		}

	}
	if (m_pWarnings[1])
		m_pWarnings[1]->SetActive(false);
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
		if (m_pWarnings[0] && m_pWarnings[1])
		{
			m_pWarnings[0]->SetActive(false);
			m_pWarnings[1]->SetActive(true);
		}

		m_bAttackedLeft = true;
		pAnimatorLeft->SetTrigger("Punch");
	}

	if (!m_bDetectedLeftt&&m_fTimeElapsed >= LEFTARM_START_TIME + 0.5f && m_bAttackedLeft && m_pWarnings[0] )
	{
		_vector vTargetPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);
		_vector vMyPos = m_pWarnings[0]->GetTransform()->Get_State(STATE::POSITION);
		_vector vDelta = vTargetPos - vMyPos;

		_float fHalfWidth = 60.f * 0.5f;   // X绵 规氢 馆气
		_float fHalfDepth = 300.f * 0.5f;   // Z绵 规氢 馆气
		_float dx = XMVectorGetX(vDelta);
		_float dz = XMVectorGetZ(vDelta);
		if (fabsf(dx) <= fHalfWidth && fabsf(dz) <= fHalfDepth)
		{
			pChar->OnAttackHit(pChar->GetTarget());
			/*		pChar->Blow(this, 40.f);
					pChar->TakeDamage(15.f);
					pChar->GetTarget()->HurtDown();
					pChar->GetTarget()->TakeDamage(7.f);
					pChar->GetTarget()->StartHitStop(0.3f);
					pChar->StartHitStop(0.3f);*/
		}

		m_bDetectedLeftt = true;
	}



	if (m_fTimeElapsed >= RIGHTARM_START_TIME && !m_bAttackedRight)
	{
		if (m_pWarnings[1])
		{
			m_pWarnings[1]->SetActive(false);
		}
		m_bAttackedRight = true;
		pAnimatorRight->SetTrigger("Punch");
		
	}
	if (!m_bDetectedRight&&m_fTimeElapsed >= RIGHTARM_START_TIME + 0.5f && m_bAttackedRight && m_pWarnings[1])
	{
		_vector vTargetPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);
		_vector vMyPos = m_pWarnings[1]->GetTransform()->Get_State(STATE::POSITION);
		_vector vDelta = vTargetPos - vMyPos;

		_float fHalfWidth = 60.f * 0.5f;   // X绵 规氢 馆气
		_float fHalfDepth = 300.f * 0.5f;   // Z绵 规氢 馆气
		_float dx = XMVectorGetX(vDelta);
		_float dz = XMVectorGetZ(vDelta);
		if (fabsf(dx) <= fHalfWidth && fabsf(dz) <= fHalfDepth)
		{
			pChar->OnAttackHit(pChar->GetTarget());
		}
		m_bDetectedRight = true;
	}


	if (m_fTimeElapsed >= ATTACK_END_TIME)
	{
		
		pChar->ChangeState(new BossIdle(TEXT("Idle")));
	}
}

void BossPunch::Exit(CEnmuMeat* pChar)
{
	auto leftArm =dynamic_cast<CEnmuArm*>(pChar->GetPart(CEnmuMeat::Parts::LEFTARM));
	auto rightArm = dynamic_cast<CEnmuArm*>(pChar->GetPart(CEnmuMeat::Parts::RIGHTARM));
	if (leftArm)
	{
		leftArm->StartRotateY(0.5f, 180.f);
	}
	if (rightArm)
	{
		rightArm->StartRotateY(0.5f, 180.f);
	}

	for (_int i = 0; i < m_pWarnings.size(); i++)
	{
		if (m_pWarnings[i])
		{
			m_pWarnings[i]->SetActive(false);
		}
	}
}
