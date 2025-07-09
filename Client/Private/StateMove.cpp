#include "StateMove.h"
#include "StateIdle.h"
#include "StateAttack1.h"
#include "StateJump.h"
#include "StateGuard.h"
#include "Collider.h"
#include "StateStep.h"
#include "StateSkill1.h"


void StateMove::Enter(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Move", true);
	auto col = static_cast<CCollider*>(pChar->Get_Component(TEXT("Com_CapsuleCollider")));
	if (col)
	{
		col->SetPriority(1); // 이동 상태에서는 충돌 우선순위를 높임
	}
	pChar->SetState(CBaseCharacter::CSTATE::MOVE);
	pChar->GetInputBuffer()->PopCommand(ECommand::Move);
	CSoundMag::Get_Instance()->PlayEffect("event:/Common/FootGrass","FootGrass");
}

void StateMove::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{

	auto buf = pChar->GetInputBuffer();
	auto anim = pChar->Get_Animator();
	if(anim->CheckBool("Stepping"))
		return;

	CGameInstance* pGameInstance = CGameInstance::Get_Instance();
	if (input.doGuard)
	{
		buf->PopCommand(ECommand::Guard, 1);
		pChar->ChangeState(new StateGuard(TEXT("Guard")));
		return;
	}

	if (input.doSkill1) // 이동 스킬
	{
		pChar->ChangeState(new StateSkill1(TEXT("Skill1")));
		return;
	}

	// 1타 공격 입력
	if (input.doAttack)
	{
	//	buf->PopFront(1);
		pChar->ChangeState(new StateAttack1(TEXT("Attack1")));
		return;
	}

	if (input.doJump)
	{
		buf->PopCommand(ECommand::Jump, 1);
		pChar->ChangeState(new StateJump(TEXT("Jump")));
		return;
	}

	auto pTarget = pChar->Get_Target();

	_bool bMoving = !XMVector3Equal(input.moveDir, XMVectorZero());

	if (bMoving && input.doStep && pTarget)
	{
		XMVECTOR dir = XMVectorZero();

		dir = XMVector3Normalize(input.moveDir);

		_vector playerPos = pChar->GetTransform()->Get_State(STATE::POSITION);
		_vector enemyPos = pTarget->GetTransform()->Get_State(STATE::POSITION);
		_vector forwardEn = XMVector3Normalize(enemyPos - playerPos);
		_vector rightEn = XMVector3Normalize(XMVector3Cross(XMVectorSet(0, 1, 0, 0), forwardEn));

		_float fDot = XMVectorGetX(XMVector3Dot(forwardEn, dir));
		_float rDot = XMVectorGetX(XMVector3Dot(rightEn, dir));
		const _float dead = 0.3f;

		EDirection basicDir = EDirection::NONE;
		if (fabsf(fDot) > fabsf(rDot))
		{
			if (fDot > dead) basicDir = EDirection::Forward;
			else if (fDot < -dead) basicDir = EDirection::Backward;
		}
		else
		{
			if (rDot > dead) basicDir = EDirection::Right;
			else if (rDot < -dead) basicDir = EDirection::Left;
		}
		if (basicDir == EDirection::NONE)
			return;

		EDirection last = pChar->GetLastStepDirection();
		EDirection finalDir = basicDir;
		buf->PopCommand(ECommand::Dash, 1);
		pChar->SetLastStepDirection(finalDir);  // 마지막 스텝 방향 갱신
		pChar->ChangeState(new StateStep(TEXT("Step"),finalDir));
		return;
	}

	if (bMoving&&!anim->CheckBool("Attacking")&&!anim->CheckBool("Jump"))
	{
		auto pTransform =  pChar->GetTransform();
		pTransform->RotateToDirection(input.moveDir);
		pTransform->MoveDirection(input.moveDir, fTimeDelta,pChar->GetNavigation());

	}
	else
	{
		pChar->ChangeState(new StateIdle(TEXT("Idle")));
		return;
	}
}

void StateMove::Exit(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Move", false);
	auto col = static_cast<CCollider*>(pChar->Get_Component(TEXT("Com_CapsuleCollider")));
	if (col)
	{
		col->SetPriority(0); 
	}
	CSoundMag::Get_Instance()->StopEffect("FootGrass");
}
