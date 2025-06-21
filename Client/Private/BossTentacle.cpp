#include "BossTentacle.h"
#include "BossIdle.h"

void BossTentacle::Enter(CEnmuMeat* pChar)
{
	auto pAnimatorLeft = pChar->GetPart(CEnmuMeat::Parts::LEFTARM)->Get_Animator();
	auto pAnimatorRight = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM)->Get_Animator();

	pAnimatorLeft->SetTrigger("TentacleAttackStart");
	pAnimatorRight->SetTrigger("TentacleAttackStart");
	pAnimatorLeft->SetBool("TentacleAttackEnd", false);
	pAnimatorRight->SetBool("TentacleAttackEnd", false);
	pChar->SetState(EnmuState::TENTACLEATTACK);
	m_iTentacleIndex = 0;
	m_fTimeElapsed = 0.f; // 시간 초기화
	m_fAttackTimeElapsed = 0.f; // 공격 시간 초기화
	m_fWaringTimeElapsed = 0.f; // 경고 시간 초기화
	m_bAttackPrepared = false; // 공격 준비 상태 초기화
	m_fNextAttackTime = 1.5f; // 다음 공격 시간 초기화
	pChar->ResetTentacles(); // 촉수 초기화
}

void BossTentacle::Update(CEnmuMeat* pChar, _float fTimeDelta)
{
	auto pAnimatorLeft = pChar->GetPart(CEnmuMeat::Parts::LEFTARM)->Get_Animator();
	auto pAnimatorRight = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM)->Get_Animator();
	m_fTimeElapsed += fTimeDelta;
	m_fAttackTimeElapsed += fTimeDelta;

	if (pChar->GetTarget())
	{
		_vector vTargetPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);
		_float y = XMVectorGetY(vTargetPos);
		vTargetPos = XMVectorSet(XMVectorGetX(vTargetPos), 0.f, XMVectorGetZ(vTargetPos), 1.f); // y축을 0으로 설정
		_vector vMyPos = pChar->GetPart(CEnmuMeat::Parts::BODY)->GetTransform()->Get_State(STATE::POSITION);
		y = -18.f; // 
		vMyPos = XMVectorSet(XMVectorGetX(vMyPos), y, XMVectorGetZ(vMyPos), 1.f); // y축을 0으로 설정
		_vector vDir = XMVector3Normalize(vTargetPos - vMyPos);

		if (m_fNextAttackTime <= m_fAttackTimeElapsed) // 다음 공격 쿨타임 지났으면
		{
			m_fWaringTimeElapsed += fTimeDelta; // 경고 시간 늘려주고
			if (WARING_TIME <= m_fWaringTimeElapsed)
			{
				pChar->ActiveTentacle(m_iTentacleIndex, true); // 촉수 활성화
				m_bAttackPrepared = false; // 공격 준비 상태 초기화
				m_fWaringTimeElapsed = 0.f;
				m_fAttackTimeElapsed = 0.f;
			}
			else
			{
				if (!m_bAttackPrepared)
				{
					pChar->DestroyTentacle(m_iTentacleIndex); // 기존 촉수 제거
					m_iTentacleIndex = (m_iTentacleIndex + 1) % 10;
					pChar->SpawnTentacle(m_iTentacleIndex, vTargetPos, vDir); // 촉수 소환
					pChar->ActiveTentacle(m_iTentacleIndex, false); // 촉수 비활성화
					m_bAttackPrepared = true; // 공격 준비 완료상태
				}
			}
		}
	}
	
	if (m_fTimeElapsed >= ATTACK_END_TIME)
	{
		pAnimatorLeft->SetBool("TentacleAttackEnd", true);
		pAnimatorRight->SetBool("TentacleAttackEnd", true);
		pChar->ResetTentacles();
		pChar->ChangeState(new BossIdle(TEXT("Idle")));
	}
}

void BossTentacle::Exit(CEnmuMeat* pChar)
{

}
