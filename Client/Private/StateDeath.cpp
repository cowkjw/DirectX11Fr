#include "StateDeath.h"

void StateDeath::Enter(CBaseCharacter* pChar)
{
	pChar->SetState(CBaseCharacter::CSTATE::DIE);
	pChar->DeactiveCollider();
	auto pModel = dynamic_cast<CModel*>(pChar->Get_Component(TEXT("Com_Model")));
	pModel->GetAnimationClipByName("A_P0000_V00_C00_Death")->SetLoop(false);
	pChar->Get_Animator()->PlayClip(pModel->GetAnimationClipByName("A_P0000_V00_C00_Death"),false);
	pChar->Get_Animator()->SetBool("Death", false);
}

void StateDeath::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	auto pAnim = pChar->Get_Animator();
//	const string& currentAnim = pAnim->GetCurrentAnimName();
	if (pAnim->CheckBool("Death"))
	{
		pAnim->StopAnimation();
		pChar->SetState(CBaseCharacter::CSTATE::DIE);
	}
}

void StateDeath::Exit(CBaseCharacter* pChar)
{
}
