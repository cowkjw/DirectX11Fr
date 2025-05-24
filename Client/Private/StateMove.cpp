#include "StateMove.h"
#include "Animator.h"
#include "GameInstance.h"
#include "BaseCharacter.h"
#include "AnimController.h"
#include "StateIdle.h"
#include "StateAttack1.h"
#include "StateJump.h"
#include "StateGuard.h"
#include "StateSkill1.h"


void StateMove::Enter(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Move", true);
}

void StateMove::Update(CBaseCharacter* pChar, _float fTimeDelta)
{

	auto buf = pChar->GetInputBuffer();
	auto anim = pChar->Get_Animator();

	CGameInstance* pGameInstance = CGameInstance::Get_Instance();
	if (pGameInstance->IsKeyPressed('O'))
	{
		pChar->ChangeState(new StateGuard());
		return;
	}

	if (pGameInstance->IsKeyPressed('I'))
	{
		pChar->ChangeState(new StateSkill1());
		return;
	}

	// 1타 공격 입력
	if (buf->CheckCommand(ECommand::LightAttack))
	{
		buf->PopFront(1);
		pChar->ChangeState(new StateAttack1());
		return;
	}

	if (pGameInstance->IsKeyPressed('K'))
	{
		pChar->ChangeState(new StateJump());
		return;
	}

	_vector dir = XMVectorZero();
	if (pGameInstance->IsKeyDown(VK_UP))    dir += XMVectorSet(0, 0, 1, 0);
	if (pGameInstance->IsKeyDown(VK_DOWN))  dir += XMVectorSet(0, 0, -1, 0);
	if (pGameInstance->IsKeyDown(VK_LEFT))  dir += XMVectorSet(-1, 0, 0, 0);
	if (pGameInstance->IsKeyDown(VK_RIGHT)) dir += XMVectorSet(1, 0, 0, 0);

	_bool bMoving = !XMVector3Equal(dir, XMVectorZero());

	if (bMoving&&!anim->CheckBool("Attacking")&&!anim->CheckBool("Jump"))
	{
		auto pTransform =  pChar->GetTransform();
		pTransform->RotateToDirection(dir);
		pTransform->Go_Straight(fTimeDelta);

	}
	else
	{
		pChar->ChangeState(new StateIdle());
		return;
	}
}

void StateMove::Exit(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Move", false);
}
