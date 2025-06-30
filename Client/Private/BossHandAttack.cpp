#include "BossHandAttack.h"
#include "BossIdle.h"
#include <EnmuArm.h>
#include "ThirdPersonCamera.h"

void BossHandAttack::Enter(CEnmuMeat* pChar)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();
	leftArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, 225.f, 0.f));  
	rightArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, 135.f, 0.f));
	pAnimatorLeft->SetTrigger("HandAttackStart");
	pAnimatorRight->SetTrigger("HandAttackStart");

	pChar->SetState(EnmuState::HANDATTACK);

	m_bAttackedLeft = false;
	m_bCheckedLeft = false;
	m_bAttackedRight = false;
	m_bCheckedRight = false;
	m_fTimeElapsed = 0.f;

	for (_int i = 0; i < m_pWarnings.size(); i++)
	{
		auto pWarning = CGameInstance::Get_Instance()->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_WarningZone"), ToIndex(LEVEL::ENMU_BOSS)
			, TEXT("WarningZone"));
		if (pWarning)
		{
			if (i == 0)
			{
				pWarning->GetTransform()->Set_State(STATE::POSITION, XMVectorSet(15.f, -17.772f, -85.248f, 1.f));
			}
			else if (i == 1)
			{
				pWarning->GetTransform()->Set_State(STATE::POSITION, XMVectorSet(-2.230f, -17.772f, -85.248f, 1.f));
			}
			pWarning->GetTransform()->Scaling(_float3(80.f, 80.f, 80.f));
			m_pWarnings[i] = pWarning;
		}
	
	}
	if(m_pWarnings[1])
		m_pWarnings[1]->SetActive(false);
	m_fTimeElapsed = 0.f;
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
			if (m_pWarnings[0])
			{
				m_pWarnings[0]->SetActive(false); // 경고존 비활성화
			}
			m_pWarnings[1]->SetActive(true);
		}
		m_bAttackedLeft = true;
		pAnimatorLeft->SetTrigger("HandAttack");

	}


	if (m_bAttackedLeft && m_fTimeElapsed >= m_fLeftDetectTime&&!m_bCheckedLeft)
	{
		if(m_pWarnings[0])
		{
			if (pChar->GetTarget())
			{
				// 원 안에 있었는지 
				_vector vTargetPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);
				_vector vMyPos = m_pWarnings[0]->GetTransform()->Get_State(STATE::POSITION);
				_vector vDelta = vTargetPos - vMyPos;

				// 거리 제곱 계산
				_float fDistSq = XMVectorGetX(XMVector3LengthSq(vDelta));
				_float radiusSq = 40.f * 40.f;

				if (fDistSq <= radiusSq)
				{
					pChar->GetTarget()->HurtDown();
					pChar->GetTarget()->TakeDamage(7.f);
					pChar->GetTarget()->StartHitStop(0.3f); // 히트스톱 시작
					pChar->StartHitStop(0.3f); // 캐릭터도 히트스톱 시작

				}
			}
		}
		m_bCheckedLeft = true; // 왼팔 공격 후 체크 완료
	}

	if (m_fTimeElapsed >= RIGHTARM_START_TIME && !m_bAttackedRight)
	{
		if (m_pWarnings[1])
		{
			m_pWarnings[1]->SetActive(false);
			if (m_pWarnings[1])
			{
				m_pWarnings[1]->SetActive(false); // 경고존 비활성화
			}
		}
		m_bAttackedRight = true;
		pAnimatorRight->SetTrigger("HandAttack");
	}
	if (m_bAttackedRight && m_fTimeElapsed >= m_fRightDetectTime&&!m_bCheckedRight)
	{
		if (m_pWarnings[1])
		{
			if (pChar->GetTarget())
			{
				// 원 안에 있었는지 
				_vector vTargetPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);
				_vector vMyPos = m_pWarnings[1]->GetTransform()->Get_State(STATE::POSITION);
				_vector vDelta = vTargetPos - vMyPos;
				// 거리 제곱 계산
				_float fDistSq = XMVectorGetX(XMVector3LengthSq(vDelta));
				_float radiusSq = 40.f * 40.f;
				if (fDistSq <= radiusSq)
				{
					pChar->GetTarget()->HurtDown();
					pChar->GetTarget()->TakeDamage(7.f);
				}
			}
		}
		m_bCheckedRight = true; // 오른팔 공격 후 체크 완료
	}
	if (m_fTimeElapsed >= ATTACK_END_TIME)
	{
		
		pChar->ChangeState(new BossIdle(TEXT("Idle")));
		return;
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

	for (_int i = 0; i < m_pWarnings.size(); i++)
	{
		if (m_pWarnings[i])
		{
			m_pWarnings[i]->SetActive(false);
		}
	}
}
