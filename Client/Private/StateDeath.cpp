#include "StateDeath.h"

void StateDeath::Enter(CBaseCharacter* pChar)
{
	pChar->SetState(CBaseCharacter::CSTATE::DIE);
	pChar->DeactiveCollider();
	auto pModel = dynamic_cast<CModel*>(pChar->Get_Component(TEXT("Com_Model")));
	pModel->GetAnimationClipByName("A_P0000_V00_C00_Death")->SetLoop(false);
	pChar->Get_Animator()->PlayClip(pModel->GetAnimationClipByName("A_P0000_V00_C00_Death"));
}

void StateDeath::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	auto pAnim = pChar->Get_Animator();

	if (pAnim->GetCurrentAnimProgress() >= 1.f)
	{
		pAnim->StopAnimation();
	}
}

void StateDeath::Exit(CBaseCharacter* pChar)
{
}
