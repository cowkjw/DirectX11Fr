#include "StateAttack2.h"
#include "StateIdle.h"
#include "StateMove.h"
#include "StateSkill0.h"
#include "StateSkill1.h"
#include "StateAttack3.h"

void StateAttack2::Enter(CBaseCharacter* pChar)
{
    pChar->Get_Animator()->SetTrigger("Attack");
    pChar->Get_Animator()->SetBool("Attacking", true);
    pChar->Get_Animator()->SetBool("Move", false);
}

void StateAttack2::Update(CBaseCharacter* pChar, _float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto pAnim = pChar->Get_Animator();
	auto buf = pChar->GetInputBuffer();


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


	if (buf->CheckCombo({ ECommand::LightAttack, ECommand::LightAttack }))
	{
		buf->PopFront(2);
		bIsCombo = true;
		pChar->ChangeState(new StateAttack3());
		return;
	}
	auto animCtrl = pAnim->GetAnimController();
	const string& stateName = animCtrl->GetCurrentState()->stateName;
	if (stateName == "attack1" && pAnim->GetCurrentAnimProgress() >= 1.f)
	{
		_bool moving = gi->IsKeyDown(VK_UP) || gi->IsKeyDown(VK_DOWN) ||
			gi->IsKeyDown(VK_LEFT) || gi->IsKeyDown(VK_RIGHT);
		bIsCombo = false;
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
	bIsCombo = true;
}

void StateAttack2::Exit(CBaseCharacter* pChar)
{
    pChar->Get_Animator()->SetBool("Attacking", bIsCombo);
}
