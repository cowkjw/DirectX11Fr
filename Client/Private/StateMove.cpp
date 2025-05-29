#include "StateMove.h"
#include "StateIdle.h"
#include "StateAttack1.h"
#include "StateJump.h"
#include "StateGuard.h"
#include "StateStep.h"
#include "StateSkill1.h"


void StateMove::Enter(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Move", true);
}

void StateMove::Update(CBaseCharacter* pChar, _float fTimeDelta)
{

	auto buf = pChar->GetInputBuffer();
	auto anim = pChar->Get_Animator();
	if(anim->CheckBool("Stepping"))
		return;

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

	auto pTarget = pChar->Get_Target();

	_vector dir = XMVectorZero();
	if (pGameInstance->IsKeyDown(VK_UP))
	{
		dir += XMVectorSet(0, 0, 1, 0);

	}
	if (pGameInstance->IsKeyDown(VK_DOWN))
	{
		dir += XMVectorSet(0, 0, -1, 0);

	}
	if (pGameInstance->IsKeyDown(VK_LEFT))
	{
		dir += XMVectorSet(-1, 0, 0, 0);
	
	}
	if (pGameInstance->IsKeyDown(VK_RIGHT))
	{
		dir += XMVectorSet(1, 0, 0, 0);
	}

	_bool bMoving = !XMVector3Equal(dir, XMVectorZero());

	if (bMoving && pGameInstance->IsKeyDown('L') && pTarget)
	{
		//EDirection stepDir = EDirection::NONE; // 기본값

		//// 이전에 좌우면 해당 방향으로 이동해야하니까 검사 먼저
		//if (pChar->GetLastStepDirection() != EDirection::Left2 && pChar->GetLastStepDirection() == EDirection::Left)
		//{

		//	stepDir = EDirection::Left2;
		//	pChar->ChangeState(new StateStep(stepDir));
		//	return;

		//}
		//else if (pChar->GetLastStepDirection() != EDirection::Right2 && pChar->GetLastStepDirection() == EDirection::Right)
		//{
		//	stepDir = EDirection::Right2;
		//	pChar->ChangeState(new StateStep(stepDir));
		//	return;
		//}

		//// 플레이어 적 전방 벡터 (월드 기준, 길이 1)
		//_vector playerPos = pChar->GetTransform()->Get_State(STATE::POSITION);
		//_vector enemyPos = pTarget->GetTransform()->Get_State(STATE::POSITION);
		//_vector forwardEnemy = XMVector3Normalize(enemyPos - playerPos);

		////전방과 직교하는 우 방향 벡터 계산
		//_vector up = XMVectorSet(0, 1, 0, 0);
		//_vector rightEnemy = XMVector3Normalize(XMVector3Cross(up, forwardEnemy));

	
		////    전방 성분, 우측 성분 구하기
		//dir = XMVector3Normalize(dir);  // 입력 방향 정규화
		//_float fDot = XMVectorGetX(XMVector3Dot(forwardEnemy, dir));  // +면 앞, 면 뒤-
		//_float rDot = XMVectorGetX(XMVector3Dot(rightEnemy, dir));  // +면 우,  면 좌-


		//const _float dead = 0.3f;

		//if (fabsf(fDot) > fabsf(rDot))
		//{

		//	if (fDot > dead)
		//		stepDir = EDirection::Forward;
		//	else if (fDot < -dead)
		//		stepDir = EDirection::Backward;
		//	else                  
		//		return;  // 입력 작아서 스텝 없음
		//}
		//else
		//{
	
		//	if (rDot > dead)
		//	{
		//		stepDir = EDirection::Right;
		//		/*if (pChar->GetLastStepDirection() != EDirection::Right2 && pChar->GetLastStepDirection() == EDirection::Right)
		//		{

		//			stepDir = EDirection::Right2;
		//		}
		//		else
		//		{
		//			stepDir = EDirection::Right;
		//		}*/

		//	}
		//	else if (rDot < -dead)
		//	{
		//		stepDir = EDirection::Left;
		///*		if (pChar->GetLastStepDirection() != EDirection::Left2 && pChar->GetLastStepDirection() == EDirection::Left)
		//		{

		//			stepDir = EDirection::Left2;
		//		}
		//		else
		//		{
		//		    stepDir = EDirection::Left;
		//		}*/
		//	}
		//	else                   
		//		return;
		//}
		//if (stepDir == EDirection::NONE)
		//	return;
		//pChar->ChangeState(new StateStep(stepDir));
		//return;
		XMVECTOR dir = XMVectorZero();
		if (pGameInstance->IsKeyDown(VK_UP))    dir += XMVectorSet(0, 0, 1, 0);
		if (pGameInstance->IsKeyDown(VK_DOWN))  dir += XMVectorSet(0, 0, -1, 0);
		if (pGameInstance->IsKeyDown(VK_LEFT))  dir += XMVectorSet(-1, 0, 0, 0);
		if (pGameInstance->IsKeyDown(VK_RIGHT)) dir += XMVectorSet(1, 0, 0, 0);
		if (XMVector3Equal(dir, XMVectorZero()))
			return;  // 방향 입력이 작으면 스텝 취소

		dir = XMVector3Normalize(dir);

		// 1) 적 상대 전방/우 방향
		_vector playerPos = pChar->GetTransform()->Get_State(STATE::POSITION);
		_vector enemyPos = pTarget->GetTransform()->Get_State(STATE::POSITION);
		_vector forwardEn = XMVector3Normalize(enemyPos - playerPos);
		_vector rightEn = XMVector3Normalize(XMVector3Cross(XMVectorSet(0, 1, 0, 0), forwardEn));

		// 2) 기본 방향 판정
		float fDot = XMVectorGetX(XMVector3Dot(forwardEn, dir));
		float rDot = XMVectorGetX(XMVector3Dot(rightEn, dir));
		const float dead = 0.3f;

		EDirection basicDir = EDirection::NONE;
		if (fabsf(fDot) > fabsf(rDot)) {
			if (fDot > dead) basicDir = EDirection::Forward;
			else if (fDot < -dead) basicDir = EDirection::Backward;
		}
		else {
			if (rDot > dead) basicDir = EDirection::Right;
			else if (rDot < -dead) basicDir = EDirection::Left;
		}
		if (basicDir == EDirection::NONE)
			return;

		// 3) 토글 로직 (같은 basicDir이 연속되면 2단계 상태로)
		EDirection last = pChar->GetLastStepDirection();
		EDirection finalDir = basicDir;
		//if (basicDir == EDirection::Left) {
		//	if (last == EDirection::Left)  finalDir = EDirection::Left2;   // 1단 → 2단
		//	else if (last == EDirection::Left2) finalDir = EDirection::Left;    // 2단 → 1단
		//	else                                 finalDir = EDirection::Left;    // 처음 또는 다른 방향 → 1단
		//}
		//else if (basicDir == EDirection::Right) {
		//	if (last == EDirection::Right)  finalDir = EDirection::Right2;  // 1단 → 2단
		//	else if (last == EDirection::Right2) finalDir = EDirection::Right;   // 2단 → 1단
		//	else                                  finalDir = EDirection::Right;   // 처음 또는 다른 방향 → 1단
		//}
		// 4) 상태 전이
		pChar->SetLastStepDirection(finalDir);  // 마지막 스텝 방향 갱신
		pChar->ChangeState(new StateStep(finalDir));
		return;
	}

	if (bMoving&&!anim->CheckBool("Attacking")&&!anim->CheckBool("Jump"))
	{
		auto pTransform =  pChar->GetTransform();
		pTransform->RotateToDirection(dir);
		pTransform->MoveDirection(dir, fTimeDelta);

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
