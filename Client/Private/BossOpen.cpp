#include "BossOpen.h"
#include "BossIdle.h"
#include <ThirdPersonCamera.h>

void BossOpen::Enter(CEnmuMeat* pChar)
{
	auto body = pChar->GetPart(CEnmuMeat::Parts::BODY);
	body->DeactiveCollider();
	auto pAnimator = body->Get_Animator();
	pAnimator->SetTrigger("OpenStart");
	if (!m_bIsOpen)
	{
		m_bIsOpen = true;
		pAnimator->SetBool("Open", m_bIsOpen);
	}
	auto head = pChar->GetPart(CEnmuMeat::Parts::HEAD);
	head->ActiveCollider();
	pChar->SetState(EnmuState::OPEN);

	if (auto pCamera = dynamic_cast<CThirdPersonCamera*>(CGameInstance::Get_Instance()->Find_GameObjectByName(ToIndex(LEVEL::ENMU_BOSS), TEXT("ThirdPersonCamera"))))
	{

		auto pTan = pChar->GetTarget();

		if (pTan)
		{
			// 오프셋 계산
			_vector vCamPos{ 60.604f,15.f,-80.332f,1.f };
			pCamera->OnHit(pTan, pChar, vCamPos, m_fOpenDuration);
		}
	}
}

void BossOpen::Update(CEnmuMeat* pChar, _float fTimeDelta)
{

	auto body = pChar->GetPart(CEnmuMeat::Parts::BODY);

	auto pAnimator = body->Get_Animator();

	if (m_bIsOpen)
	{
		m_fOpenTime += fTimeDelta;

		if (m_fOpenDuration <= m_fOpenTime)
		{
			auto head = pChar->GetPart(CEnmuMeat::Parts::HEAD);
			head->DeactiveCollider();
			m_bIsOpen = false;
			pAnimator->SetBool("Open", m_bIsOpen);
			pChar->ChangeState(new BossIdle(TEXT("Idle")));
		}
	}
}

void BossOpen::Exit(CEnmuMeat* pChar)
{
	auto body = pChar->GetPart(CEnmuMeat::Parts::BODY);
	auto pAnimator = body->Get_Animator();
	pChar->SetState(EnmuState::CLOSING);
	body->ActiveCollider();
	auto head = pChar->GetPart(CEnmuMeat::Parts::HEAD);
	head->DeactiveCollider();
}
