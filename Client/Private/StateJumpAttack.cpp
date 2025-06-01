#include "StateJumpAttack.h"
#include "StateIdle.h"
#include "StateMove.h"
#include "StateJump.h"

void StateJumpAttack::Enter(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetTrigger("JumpAttack");
	pAnim->SetBool("Attacking", true);
	pAnim->SetBool("Move", false);
	pAnim->SetBool("Jump", true);
	pChar->SetState(CBaseCharacter::CSTATE::ATTACK);
}

void StateJumpAttack::Update(CBaseCharacter* pChar, _float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto pAnim = pChar->Get_Animator();
	auto ctrl = pAnim->GetAnimController();
	const string& stateName = ctrl->GetCurrentState()->stateName;
	if (stateName.find("jumpAttack")!=string::npos&&pAnim->GetCurrentAnimProgress() >= 1.f)
	{
		pChar->ChangeState(new StateJump(TEXT("Jump")));
		return;

		///*_bool moving = gi->IsKeyDown(VK_UP) || gi->IsKeyDown(VK_DOWN) ||
		//	gi->IsKeyDown(VK_LEFT) || gi->IsKeyDown(VK_RIGHT);
		//if (moving)
		//{
		//	pChar->ChangeState(new StateMove());
		//	return;
		//}
		//else
		//{
		//	pChar->ChangeState(new StateIdle());
		//	return;
		//}
		//return;*/
	}
}

void StateJumpAttack::Exit(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Attacking", false);
}
