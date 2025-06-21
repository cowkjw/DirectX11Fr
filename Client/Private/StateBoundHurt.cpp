#include "StateBoundHurt.h"
#include "StateIdle.h"
#include "StateMove.h"

void StateBoundHurt::Enter(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Hurted", true);
	pAnim->SetTrigger("HurtBound");
	pAnim->SetBool("Move", false);
	pChar->SetState(CBaseCharacter::CSTATE::BOUND);

}

void StateBoundHurt::Update(CBaseCharacter* pChar, const InputData& input, _float fTimeDelta)
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

void StateBoundHurt::Exit(CBaseCharacter* pChar)
{
}
