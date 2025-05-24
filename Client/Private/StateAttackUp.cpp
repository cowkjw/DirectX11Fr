#include "StateAttackUp.h"
#include "StateIdle.h"
#include "StateMove.h"
#include "StateSkill0.h"
#include "StateSkill1.h"

void StateAttackUp::Enter(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Attacking", true);
	pAnim->SetTrigger("AttackUp");
	pAnim->SetBool("Move", false);
	pAnim->SetBool("Jump", false);
}

void StateAttackUp::Update(CBaseCharacter* pChar, _float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto anim = pChar->Get_Animator();
	auto animCtrl = anim->GetAnimController();
	auto buf = pChar->GetInputBuffer();
	const string& stateName = animCtrl->GetCurrentState()->stateName;

	if ((gi->IsKeyDown(VK_UP) || gi->IsKeyDown(VK_DOWN) ||
		gi->IsKeyDown(VK_LEFT) || gi->IsKeyDown(VK_RIGHT)) && gi->IsKeyDown('I'))
	{

		pChar->ChangeState(new StateSkill1());
		return;
	}

	if (buf->CheckCommand(ECommand::Skill0)) {
		buf->PopFront(1);
		pChar->ChangeState(new StateSkill0());
		return;
	}
	if (stateName == "attackUp" && anim->GetCurrentAnimProgress() >= 1.f)
	{
		bIsCombo = false;
		_bool moving = gi->IsKeyDown(VK_UP) || gi->IsKeyDown(VK_DOWN) ||
			gi->IsKeyDown(VK_LEFT) || gi->IsKeyDown(VK_RIGHT);
		pChar->GetInputBuffer()->ClearBuffer();
		if (moving)
		{
			pChar->ChangeState(new StateMove());
		}
		else
		{
			pChar->ChangeState(new StateIdle());
		}
		return;

	}

}

void StateAttackUp::Exit(CBaseCharacter* pChar)
{
	
}
