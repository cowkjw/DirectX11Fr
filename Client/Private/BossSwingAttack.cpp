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

	if (!pWarning)
	{
		pWarning = CGameInstance::Get_Instance()->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_WarningZone"), ToIndex(LEVEL::ENMU_BOSS)
			, TEXT("WarningZone"));
		pWarning->GetTransform()->Set_State(STATE::POSITION, XMVectorSet(18.230f, -17.772f, -71.248f, 1.f));
		pWarning->GetTransform()->Scaling(_float3(352.f, 352.f, 352.f));
	}
	else
	{
		pWarning->SetActive(true);
	}
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
		if (pWarning)
		{
			pWarning->SetActive(false); // 경고존 비활성화

			if (pChar->GetTarget())
			{
				// 원 안에 있었는지 
				_vector vTargetPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);
				_vector vMyPos = pWarning->GetTransform()->Get_State(STATE::POSITION);
				_vector vDelta = vTargetPos - vMyPos;

				// 거리 제곱 계산
				_float fDistSq = XMVectorGetX(XMVector3LengthSq(vDelta));
				_float radiusSq = 176.f * 176.f;

				if (fDistSq <= radiusSq)
				{
					pChar->GetTarget()->StartHitStop(0.5f);
					pChar->GetTarget()->TakeDamage(20.f);
					pChar->GetTarget()->Blow(pChar, 40.f);
				}

			}
		}
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
