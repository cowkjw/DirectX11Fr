#include "StateSkill0.h"
#include "StateIdle.h"
#include "StateMove.h"

void StateSkill0::Enter(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Attacking", true);
	pAnim->SetTrigger("Skill0");
	pAnim->SetBool("Move", false);
	pAnim->SetBool("Jump", false);

	pChar->SetState(CBaseCharacter::CSTATE::SKILL);


}

void StateSkill0::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto anim = pChar->Get_Animator();
	auto animCtrl = anim->GetAnimController();
	const string& stateName = animCtrl->GetCurrentState()->stateName;

	if (pChar->Get_Target())
	{
		pChar->GetTransform()->LookAtXZ(pChar->Get_Target()->GetTransform()->Get_State(STATE::POSITION));
	}
	if (anim->CheckBool("Attacking") == false)
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

void StateSkill0::Exit(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();

	pAnim->SetBool("Attacking", false);
}
