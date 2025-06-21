#include "StateHurtDown.h"
#include "StateIdle.h"
#include "StateMove.h"

void StateHurtDown::Enter(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Hurted", true);
	pAnim->SetTrigger("HurtDown");
	pAnim->SetBool("Move", false);
	pChar->SetState(CBaseCharacter::CSTATE::DOWN);
}

void StateHurtDown::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	auto pAnim = pChar->Get_Animator();
	auto animCtrl = pAnim->GetAnimController();
	const string& stateName = animCtrl->GetCurrentState()->stateName;
	if (pAnim->CheckBool("Hurted") == false)
	{
		_bool bMoving = !XMVector3Equal(input.moveDir, XMVectorZero());
		if (bMoving)
		{
			pChar->ChangeState(new StateMove(TEXT("Move")));
		}
		else
		{
			pChar->ChangeState(new StateIdle(TEXT("Idle")));
		}
		return;
	}
}

void StateHurtDown::Exit(CBaseCharacter* pChar)
{
}
