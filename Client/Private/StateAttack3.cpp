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
	//pChar->SetState(CBaseCharacter::CSTATE::ATTACK);
	pChar->SetState(CBaseCharacter::CSTATE::ATTACK3);

}

void StateAttack3::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto pAnim = pChar->Get_Animator();
	auto buf = pChar->GetInputBuffer();
	_float fProgress = pAnim->GetCurrentAnimProgress();


	if (fProgress <= 0.2f)
	{

		pChar->GetTransform()->Go_Straight(fTimeDelta, pChar->GetNavigation());
	}

	if (input.doSkill1)
	{
		pChar->ChangeState(new StateSkill1(TEXT("Skill1")));
		return;
	}

	if (input.doSkill0)
	{
		pChar->ChangeState(new StateSkill0(TEXT("Skill0")));
		return;
	}

	if (fProgress >= 0.7f)
	{

		if (input.doAttack4)
		{
			buf->PopCommand(ECommand::LightAttack, 2);
			bIsCombo = true;
			if (input.doAttack3Up)
			{
				pChar->ChangeState(new StateAttackUp(TEXT("AttackUp")));
				return;
			}
			else if (input.doAttack3Down)
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

			_bool bMoving = !XMVector3Equal(input.moveDir, XMVectorZero());
			bIsCombo = false;
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
}

void StateAttack3::Exit(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Attacking", bIsCombo);
}
