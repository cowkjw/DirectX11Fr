#include "StateSkill2.h"
#include "StateIdle.h"
#include "StateMove.h"

void StateSkill2::Enter(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Attacking", true);
	pAnim->SetTrigger("Skill2");
	pAnim->SetBool("Move", false);
	pAnim->SetBool("Jump", false);
}

void StateSkill2::Update(CBaseCharacter* pChar, _float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto anim = pChar->Get_Animator();
	auto animCtrl = anim->GetAnimController();
	const string& stateName = animCtrl->GetCurrentState()->stateName;
    _bool bMoving = gi->IsKeyDown(VK_UP) ||
        gi->IsKeyDown(VK_DOWN) ||
        gi->IsKeyDown(VK_LEFT) ||
        gi->IsKeyDown(VK_RIGHT);
    //anim->SetBool("Move", moving);

    _float progress = anim->GetCurrentAnimProgress();

    if (stateName == "guardSkill2" && progress >= 1.f)
    {
        pChar->GetInputBuffer()->ClearBuffer();
		if (bMoving)
		{
			pChar->ChangeState(new StateMove(TEXT("Move")));
		}
		else
		{
			pChar->ChangeState(new StateIdle(TEXT("Idle")));
		}
    }
}

void StateSkill2::Exit(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();

	pAnim->SetBool("Attacking", false);	
	pAnim->SetBool("Guard", false);	
}
