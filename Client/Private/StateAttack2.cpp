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
	pChar->SetState(CBaseCharacter::CSTATE::ATTACK);
}

void StateAttack2::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto pAnim = pChar->Get_Animator();
	auto buf = pChar->GetInputBuffer();


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

	if (pAnim->GetCurrentAnimProgress() <= 0.35f)
	{
		pChar->GetTransform()->Go_Straight(fTimeDelta);
	}


	if (input.doAttack3)
	{
		bIsCombo = true;
		buf->PopCommand(ECommand::LightAttack, 2);
		pChar->ChangeState(new StateAttack3(TEXT("Attack3")));
		return;
	}


	//if (buf->CheckCombo({ ECommand::LightAttack, ECommand::LightAttack }))
	//{
	//	buf->PopFront(2);
	//	bIsCombo = true;
	//	pChar->ChangeState(new StateAttack3());
	//	return;
	//}
	auto animCtrl = pAnim->GetAnimController();
	const string& stateName = animCtrl->GetCurrentState()->stateName;

	if (pAnim->GetCurrentAnimProgress() >= 1.f)
	{
		bIsCombo = false;
		_bool moving = !XMVector3Equal(input.moveDir, XMVectorZero());
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

void StateAttack2::Exit(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Attacking", bIsCombo);
}
