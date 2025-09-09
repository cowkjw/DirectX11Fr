#include "StateGuard.h"
#include "Animator.h"
#include "GameInstance.h"
#include "BaseCharacter.h"
#include "AnimController.h"
#include "StateIdle.h"
#include "StateMove.h"
#include "StateSkill.h"

void StateGuard::Enter(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Guard",true);

	pChar->SetState(CBaseCharacter::CSTATE::GUARD);
	pChar->GetInputBuffer()->PopCommand(ECommand::Guard); // 가드 명령 초기화
}

void StateGuard::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	CGameInstance* pGameInstance = CGameInstance::Get_Instance();
	auto buf = pChar->GetInputBuffer();
	if (pChar->Get_Target())
	{
		pChar->GetTransform()->LookAtXZ(pChar->Get_Target()->GetTransform()->Get_State(STATE::POSITION));
	}

	if (input.doSkill2)
	{
		//buf->PopCommand(ECommand::Skill2, 1);
		pChar->ChangeState(new StateSkill(TEXT("Skill2"), StateSkill::ESkillType::Skill2));
		return;
	}

	if (!XMVector3Equal(input.moveDir, XMVectorZero()))
	{
		pChar->ChangeState(new StateMove(TEXT("Move")));
		return;
	}

}

void StateGuard::Exit(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Guard", false);
}
