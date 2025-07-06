#include "BossDeath.h"
#include "CameraMag.h"
#include "CutSceneCamera.h"

void BossDeath::Enter(CEnmuMeat* pChar)
{
	auto body = pChar->GetPart(CEnmuMeat::Parts::BODY);
	auto pAnimator = body->Get_Animator();
	pAnimator->SetTrigger("OpenStart");
	if (!m_bIsOpen)
	{
		m_bIsOpen = true;
		pAnimator->SetBool("Open", m_bIsOpen);
	}
	auto head = pChar->GetPart(CEnmuMeat::Parts::HEAD);
	head->ActiveCollider();
	head->Get_Animator()->SetTrigger("Death");
	// ÅºÁö·Î Àá±ñ ¾Èº¸ÀÌ°Ô
	pChar->GetTarget()->SetActive(false);

	CCameraMag::Get_Instance()->ActiveCamera(TEXT("CutSceneCamera"));
	CCameraMag::Get_Instance()->SetCutSceneProperty(TEXT("EnmuMeatDeath"));
	auto pCutSceneCam = dynamic_cast<CCutSceneCamera*>(CCameraMag::Get_Instance()->GetActiveCamera());
	if (!pCutSceneCam)
		return;
	pCutSceneCam->SetPlay(true);

}

void BossDeath::Update(CEnmuMeat* pChar, _float fTimeDelta)
{
	auto pCutSceneCam = dynamic_cast<CCutSceneCamera*>(CCameraMag::Get_Instance()->GetActiveCamera());
	auto pHead = pChar->GetPart(CEnmuMeat::Parts::HEAD);
	if (pCutSceneCam&& pCutSceneCam->IsPlaying() == false)
	{
		pChar->SetState(EnmuState::DIE);
		pChar->GetTarget()->SetActive(true);
		static_cast<CAnimator*>(pHead->Get_Component(TEXT("Com_Animator")))->StopAnimation();
		CCameraMag::Get_Instance()->ActiveCamera(TEXT("MainCamera"));
		return;
	}
}

void BossDeath::Exit(CEnmuMeat* pChar)
{
}
