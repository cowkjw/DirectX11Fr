#include "StateAttack3.h"
#include "StateAttack4.h"
#include "StateMove.h"
#include "StateAttackDown.h"
#include "StateAttackUP.h"
#include "StateSkill0.h"	
#include "StateSkill1.h"	
#include "StateIdle.h"

void StateAttack3::Enter(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetTrigger("Attack");
	pChar->Get_Animator()->SetBool("Attacking", true);
	pChar->Get_Animator()->SetBool("Move", false);
	pChar->SetState(CBaseCharacter::CSTATE::ATTACK);
}

void StateAttack3::Update(CBaseCharacter* pChar, _float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto pAnim = pChar->Get_Animator();
	auto buf = pChar->GetInputBuffer();

	if (pAnim->GetCurrentAnimProgress() <= 0.2f)
	{
		_float fProgress = pAnim->GetCurrentAnimProgress();


		pChar->GetTransform()->Go_Straight(fTimeDelta);
	}

	if ((gi->IsKeyDown(VK_UP) || gi->IsKeyDown(VK_DOWN) ||
		gi->IsKeyDown(VK_LEFT) || gi->IsKeyDown(VK_RIGHT)) && gi->IsKeyDown('I'))
	{

		pChar->ChangeState(new StateSkill1());
		return;
	}

	if (buf->CheckCommand(ECommand::Skill0)) {
		buf->PopFront(1);
		pChar->ChangeState(new StateSkill0(TEXT("Skill0")));
		return;
	}

	if (pAnim->GetCurrentAnimProgress() >= 0.8f)
	{
		if (buf->CheckCombo({ ECommand::LightAttack}))
		{
			buf->PopFront(1);
			bIsCombo = true;
			if (gi->IsKeyDown(VK_UP))
			{
				pChar->ChangeState(new StateAttackUp(TEXT("AttackUp")));
				return;
			}
			else if (gi->IsKeyDown(VK_DOWN))
			{
				pChar->ChangeState(new StateAttackDown(TEXT("AttackDown")));
				return;
			}
			else
			{
				pChar->ChangeState(new StateAttack4(TEXT("Attack4")));
				return;
			}
		}
		else
		{
			auto animCtrl = pAnim->GetAnimController();
			const string& stateName = animCtrl->GetCurrentState()->stateName;
			if (stateName == "attack2" && pAnim->GetCurrentAnimProgress() >= 1.f)
			{
				_bool moving = gi->IsKeyDown(VK_UP) || gi->IsKeyDown(VK_DOWN) ||
					gi->IsKeyDown(VK_LEFT) || gi->IsKeyDown(VK_RIGHT);
				bIsCombo = false;
				if (moving)
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
	}
}

void StateAttack3::Exit(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Attacking", bIsCombo);
}
