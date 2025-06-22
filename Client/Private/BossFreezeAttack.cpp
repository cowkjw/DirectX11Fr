#include "BossFreezeAttack.h"
#include "BossIdle.h"
#include <EnmuArm.h>

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
	m_pWarningZone = CGameInstance::Get_Instance()->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_WarningZone"), ToIndex(LEVEL::ENMU_BOSS)
		, TEXT("WarningZone"));

	if (m_pWarningZone)
	{
		m_pWarningZone->GetTransform()->Scaling(_float3(40.f, 40.f, 40.f));
	}
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
	_vector rawDir = XMVector3Normalize(vTargetPos - vMyPos);
	// 180° 회전 보정
	_vector vDir = XMVectorSetX(rawDir, -XMVectorGetX(rawDir));
	
	if (!m_bAttacked && m_fTimeElapsed < m_fFollowTime)
	{
		if (m_bAttckArm)
			leftArm->GetTransform()->RotateToDirection(vDir);
		else
			rightArm->GetTransform()->RotateToDirection(vDir);

		if (m_pWarningZone&&m_fTimeElapsed< m_fDetectTime)
		{
			_float fY = -17.772f;
			vTargetPos.m128_f32[1] = fY; // Y 좌표를 현재 캐릭터 위치로 설정
			m_pWarningZone->GetTransform()->Set_State(STATE::POSITION, vTargetPos);
		}
	}
	// 2) 추적 시간이 지났고(!m_bAttacked), 이제 한 번만 공격 트리거
	else if (!m_bAttacked && m_fTimeElapsed >= m_fFollowTime)
	{
		if (m_bAttckArm)
			pAnimatorLeft->SetTrigger("FreezeAttack");
		else
			pAnimatorRight->SetTrigger("FreezeAttack");
		m_bAttacked = true;
		if (m_pWarningZone)
		{
			m_pWarningZone->SetActive(false); // 경고존 비활성화

			if (pChar->GetTarget())
			{
				// 원 안에 있었는지 
				_vector vTargetPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);
				_vector vMyPos = m_pWarningZone->GetTransform()->Get_State(STATE::POSITION);
				_vector vDelta = vTargetPos - vMyPos;

				// 거리 제곱 계산
				_float fDistSq = XMVectorGetX(XMVector3LengthSq(vDelta));
				_float radiusSq = 20.f * 20.f;

				if (fDistSq <= radiusSq)
				{
					pChar->GetTarget()->StartHitStop(0.5f);
					pChar->GetTarget()->TakeDamage(100.f);
				}

			}
		}

	}

	if (m_fTimeElapsed >= ATTACK_END_TIME)
	{
		
		pChar->ChangeState(new BossIdle(TEXT("Idle")));
		
	}
}

void BossFreezeAttack::Exit(CEnmuMeat* pChar)
{
	auto leftArm = dynamic_cast<CEnmuArm*>(pChar->GetPart(CEnmuMeat::Parts::LEFTARM));
	auto rightArm = dynamic_cast<CEnmuArm*>(pChar->GetPart(CEnmuMeat::Parts::RIGHTARM));

	leftArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, 180.f, 0.f)); // 기본 방향으로 회전
	rightArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, 180.f, 0.f)); // 기본 방향으로 회전

	if (m_pWarningZone)
	{
		m_pWarningZone->SetActive(false); // 경고존 비활성화
	}
}
