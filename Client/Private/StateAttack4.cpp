#include "StateAttack4.h"
#include "StateMove.h"
#include "StateIdle.h"
#include "StateSkill0.h"
#include "StateSkill1.h"
#include "Akaza.h"

void StateAttack4::Enter(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetTrigger("Attack");
	pChar->Get_Animator()->SetBool("Attacking", true);
	pChar->Get_Animator()->SetBool("Move", false);
	//pChar->SetState(CBaseCharacter::CSTATE::ATTACK);
	pChar->SetState(CBaseCharacter::CSTATE::ATTACK4);


}

void StateAttack4::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto pAnim = pChar->Get_Animator();
	auto animCtrl = pAnim->GetAnimController();
	const string& stateName = animCtrl->GetCurrentState()->stateName;
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
}

void StateAttack4::Exit(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Attacking", false);

	if (auto akaza = dynamic_cast<CAkaza*>(pChar))
	{
		akaza->SetComState(CAkaza::COM_STATE::IDLE);
	}
}
