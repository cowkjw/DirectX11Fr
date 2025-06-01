#include "StateSkill1.h"
#include "StateIdle.h"
#include "StateMove.h"

void StateSkill1::Enter(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Attacking", true);
	pAnim->SetTrigger("Skill1");
	pAnim->SetBool("Move", false);
	pAnim->SetBool("Jump", false);
}

void StateSkill1::Update(CBaseCharacter* pChar, _float fTimeDelta)
{
	if (pChar->Get_Target())
	{
		pChar->GetTransform()->LookAt(pChar->Get_Target()->GetTransform()->Get_State(STATE::POSITION));
	}
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto anim = pChar->Get_Animator();
	auto animCtrl = anim->GetAnimController();
	const string& stateName = animCtrl->GetCurrentState()->stateName;
	if (stateName == "skill1End" && anim->GetCurrentAnimProgress() >= 1.f)
	{
		_bool moving = gi->IsKeyDown(VK_UP) || gi->IsKeyDown(VK_DOWN) ||
			gi->IsKeyDown(VK_LEFT) || gi->IsKeyDown(VK_RIGHT);
		pChar->GetInputBuffer()->ClearBuffer();
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

void StateSkill1::Exit(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Attacking", false);
}
