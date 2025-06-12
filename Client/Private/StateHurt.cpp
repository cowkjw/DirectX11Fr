#include "StateHurt.h"
#include "StateIdle.h"
#include "StateMove.h"

void StateHurt::Enter(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();



	pAnim->SetBool("Hurted", true);
	pAnim->SetTrigger("Hurt");
	pAnim->SetBool("Move", false);
	pChar->SetState(CBaseCharacter::CSTATE::HURT);
}

void StateHurt::Update(CBaseCharacter* pChar, const InputData& input, _float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
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
	///*if (pAnim->GetCurrentAnimProgress() >= 1.f)
	//{
	//
	//}*/
}

void StateHurt::Exit(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	//pAnim->SetBool("Hurted", false);
}
