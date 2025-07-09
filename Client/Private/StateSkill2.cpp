#include "StateSkill2.h"
#include "StateIdle.h"
#include "StateMove.h"


// 가드 스킬
void StateSkill2::Enter(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Attacking", true);
	pAnim->SetTrigger("Skill2");
	pAnim->SetBool("Move", false);
	pAnim->SetBool("Guard", true);

	pChar->SetState(CBaseCharacter::CSTATE::SKILL2);

	
}


void StateSkill2::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto anim = pChar->Get_Animator();
	auto animCtrl = anim->GetAnimController();
	const string& stateName = animCtrl->GetCurrentState()->stateName;
	
    //anim->SetBool("Move", moving);

    _float progress = anim->GetCurrentAnimProgress();

	if (anim->CheckBool("Attacking") == false)
	{
		_bool bMoving = !XMVector3Equal(input.moveDir, XMVectorZero());
		if (bMoving)
		{
			pChar->ChangeState(new StateMove(TEXT("Move")));
		}
		else
		{
			pChar->ChangeState(new StateIdle(TEXT("Idle")));
		}
    }
}

void StateSkill2::Exit(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();

	pAnim->SetBool("Attacking", false);	
	pAnim->SetBool("Guard", false);	
}
