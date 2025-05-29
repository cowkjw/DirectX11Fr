#include "StateGuard.h"
#include "Animator.h"
#include "GameInstance.h"
#include "BaseCharacter.h"
#include "AnimController.h"
#include "StateIdle.h"
#include "StateMove.h"
#include "StateSkill2.h"

void StateGuard::Enter(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Guard",true);
}

void StateGuard::Update(CBaseCharacter* pChar, _float fTimeDelta)
{
	CGameInstance* pGameInstance = CGameInstance::Get_Instance();
	auto buf = pChar->GetInputBuffer();
	if (pChar->Get_Target())
	{
		pChar->GetTransform()->LookAt(pChar->Get_Target()->GetTransform()->Get_State(STATE::POSITION));
	}
	if (pGameInstance->IsKeyDown('I')&& pGameInstance->IsKeyDown('O'))
	{
		pChar->ChangeState(new StateSkill2());
		return;
	}

	if (!pGameInstance->IsKeyDown('O'))
	{
		pChar->ChangeState(new StateIdle());
		return;
	}

}

void StateGuard::Exit(CBaseCharacter* pChar)
{
}
