#include "StateAttackDown.h"
#include "StateIdle.h"
#include "StateMove.h"
#include "StateSkill0.h"
#include "StateSkill1.h"

void StateAttackDown::Enter(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Attacking", true);
	pAnim->SetTrigger("AttackDown");
	pAnim->SetBool("Move", false);
	pAnim->SetBool("Jump", false);
	pChar->SetState(CBaseCharacter::CSTATE::ATTACK_DOWN);


}

void StateAttackDown::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto anim = pChar->Get_Animator();
	auto animCtrl = anim->GetAnimController();
	auto buf = pChar->GetInputBuffer();
	const string& stateName = animCtrl->GetCurrentState()->stateName;
	_float fProgress = anim->GetCurrentAnimProgress();
	if (fProgress <= 0.5f)
	{
		pChar->GetTransform()->Go_Straight(fTimeDelta,pChar->GetNavigation());
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

	if (anim->GetCurrentAnimProgress() >= 1.f)
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


}

void StateAttackDown::Exit(CBaseCharacter* pChar)
{
	
}
