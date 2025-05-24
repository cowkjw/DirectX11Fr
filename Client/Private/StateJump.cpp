#include "StateJump.h"
#include "StateMove.h"
#include "StateJumpAttack.h"
#include "StateIdle.h"


void StateJump::Enter(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Jump",true);
}

void StateJump::Update(CBaseCharacter* pChar, _float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto pAnim = pChar->Get_Animator();

	if (gi->IsKeyPressed('J'))
	{
		pChar->ChangeState(new StateJumpAttack());
		return;
	}
	auto animCtrl = pAnim->GetAnimController();
	const string& stateName = animCtrl->GetCurrentState()->stateName;
	if (stateName.find("Jump3") != string::npos)
	{
		_bool moving = gi->IsKeyDown(VK_UP) || gi->IsKeyDown(VK_DOWN) ||
			gi->IsKeyDown(VK_LEFT) || gi->IsKeyDown(VK_RIGHT);
		if (moving) 
		{
			pChar->ChangeState(new StateMove());
			return;
		}
		else
		{
			pChar->ChangeState(new StateIdle());
			return;
		}
	}

}

void StateJump::Exit(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Jump",false);
}
