#include "StateAttack1.h"

#include "StateIdle.h"
#include "StateMove.h"
#include "StateJump.h"
#include "StateGuard.h"
#include "StateSkill0.h"
#include "StateSkill1.h"
#include "StateAttack2.h"

void StateAttack1::Enter(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetTrigger("Attack");
	pChar->Get_Animator()->SetBool("Attacking", true);
	pChar->Get_Animator()->SetBool("Move", false);
	pChar->SetState(CBaseCharacter::CSTATE::ATTACK);


}

void StateAttack1::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto pAnim = pChar->Get_Animator();
	auto buf = pChar->GetInputBuffer();

	if (pChar->Get_Target())
	{
		pChar->GetTransform()->LookAtXZ(pChar->Get_Target()->GetTransform()->Get_State(STATE::POSITION));
	}

	if (pAnim->GetCurrentAnimProgress() <= 0.1f)
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
	//if ((gi->IsKeyDown(VK_UP) || gi->IsKeyDown(VK_DOWN) ||
	//	gi->IsKeyDown(VK_LEFT) || gi->IsKeyDown(VK_RIGHT))&&gi->IsKeyDown('I'))
	//{

	//	pChar->ChangeState(new StateSkill1(TEXT("Skill1")));
	//	return;
	//}

	if (input.doAttack2)
	{
		buf->PopCommand(ECommand::LightAttack, 2);
		pChar->ChangeState(new StateAttack2(TEXT("Attack2")));
		return;
	}


	/*if (buf->CheckCombo({ ECommand::LightAttack, ECommand::LightAttack }))
	{
		buf->PopFront(2);
		pChar->ChangeState(new StateAttack2(TEXT("Attack2")));
		return;
	}*/
	auto animCtrl = pAnim->GetAnimController();
	const string& stateName = animCtrl->GetCurrentState()->stateName;
	if (stateName == "attack0" && pAnim->GetCurrentAnimProgress() >= 1.f)
	{
		_bool moving = !XMVector3Equal(input.moveDir, XMVectorZero());
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
	bIsCombo = true;
}

void StateAttack1::Exit(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Attacking", bIsCombo);
}
