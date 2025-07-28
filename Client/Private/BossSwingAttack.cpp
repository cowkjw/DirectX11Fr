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
	CSoundMag::Get_Instance()->PlayEffect("event:/Enmu/ReadyAttack");
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
		
	}
	if (!m_bDected &&pWarning && m_fTimeElapsed >= CHARGING_TIME + m_fActiveWarningTime)
	{
		pWarning->SetActive(false); // 경고존 비활성화

		auto pTarget = pChar->GetTarget();
		if (pTarget)
		{
			// 원 안에 있었는지 
			_vector vTargetPos = pTarget->GetTransform()->Get_State(STATE::POSITION);
			_vector vMyPos = pWarning->GetTransform()->Get_State(STATE::POSITION);
			_vector vDelta = vTargetPos - vMyPos;

			// 거리 제곱 계산
			_float fDistSq = XMVectorGetX(XMVector3LengthSq(vDelta));
			constexpr _float radius = 176.f;
			constexpr _float radiusSq = radius * radius;

			if (fDistSq <= radiusSq)
			{
				if (pTarget)
				{
					pTarget->TakeDamage(10.f);
					pTarget->StartHitStop(0.65f);
					pTarget->Blow(pChar, 40.f);
				}
				pChar->StartHitStop(0.65f);
			}
		}
		m_bDected = true; // 경고존 활성화 후 한 번만
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
	if (pWarning)
	{
		pWarning->SetActive(false); // 경고존 비활성화
	}
}
