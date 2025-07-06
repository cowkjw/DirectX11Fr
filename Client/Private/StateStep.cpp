#include "StateStep.h"
#include "StateIdle.h"
#include "StateMove.h"
#include "DashSmokeEffect.h"
#include <Tanjiro.h>


void StateStep::Enter(CBaseCharacter* pChar)
{

	auto pAnim = pChar->Get_Animator();
	m_fElapsedTime = 0.f;
	pAnim->SetBool("Move", true);
	pAnim->SetBool("Stepping", true);

	if (pChar->Get_Target())
	{
		pChar->GetTransform()->LookAtXZ(pChar->Get_Target()->GetTransform()->Get_State(STATE::POSITION));
	}
	_vector forward = pChar->GetTransform()->Get_State(STATE::LOOK);
	_vector right = pChar->GetTransform()->Get_State(STATE::RIGHT);

	switch (m_eDirection)
	{
	case EDirection::Forward:
		m_vStepDir = forward;
		pAnim->SetTrigger("StepFront");
		break;
	case EDirection::Backward:
		m_vStepDir = XMVectorNegate(forward);
		pAnim->SetTrigger("StepBack");
		break;
	case EDirection::Right:
		m_vStepDir = right;
		pAnim->SetTrigger("StepRight");
		break;
	case EDirection::Right2:
		m_vStepDir = right;
		pAnim->SetTrigger("StepRight2");
		break;
	case EDirection::Left:
		m_vStepDir = XMVectorNegate(right);
		pAnim->SetTrigger("StepLeft");
		break;
	case EDirection::Left2:
		m_vStepDir = XMVectorNegate(right);
		pAnim->SetTrigger("StepLeft2");
		break;
	}
	m_vStepDir = XMVector3Normalize(m_vStepDir); // 방향 벡터 정규화

	auto pDash = pChar->GetDashSmokeEffect();
	if (pDash)
	{
		pDash->SetActive(true);
		auto vPos = pChar->GetTransform()->Get_State(STATE::POSITION);
		_vector offsetUp{};

		// 탄지로 높이가 좀 다른듯 함.
		if (dynamic_cast<CTanjiro*>(pChar))
		{
			offsetUp = XMVectorSet(0.f, XMVectorGetY(vPos) + 27.f, 0.f, 0.f);

		}
		else
		{
			offsetUp = XMVectorSet(0.f, XMVectorGetY(vPos) + 8.f, 0.f, 0.f);

		}
		vPos = XMVectorAdd(vPos, offsetUp);

		pDash->GetTransform()->Set_State(STATE::POSITION, vPos);

		// 플레이어 위치에서 스텝 할 방향으로 바라보게
		pDash->GetTransform()->LookAtXZ(pChar->GetTransform()->Get_State(STATE::POSITION) + m_vStepDir);
	}
}

void StateStep::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	auto buf = pChar->GetInputBuffer();
	auto anim = pChar->Get_Animator();
	auto pDash = pChar->GetDashSmokeEffect();
	if (pDash)
	{
		pDash->Update(fTimeDelta);
		pDash->Late_Update(fTimeDelta);
	}

	CGameInstance* pGameInstance = CGameInstance::Get_Instance();
	//if (pGameInstance->IsKeyPressed('O'))
	//{
	//	pChar->ChangeState(new StateGuard());
	//	return;
	//}

	//if (pGameInstance->IsKeyPressed('I'))
	//{
	//	pChar->ChangeState(new StateSkill1());
	//	return;
	//}

	//// 1타 공격 입력
	//if (buf->CheckCommand(ECommand::LightAttack))
	//{
	//	buf->PopFront(1);
	//	pChar->ChangeState(new StateAttack1());
	//	return;
	//}

	//if (pGameInstance->IsKeyPressed('K'))
	//{
	//	pChar->ChangeState(new StateJump());
	//	return;
	//}

	m_fElapsedTime += fTimeDelta;
	_float t = m_fElapsedTime / m_fStepDuration;
	// t ≥ 1이면 스텝 완료 → Idle 상태로 전환
	/*if (t >= 1.f)
	{
		pChar->Get_Animator()->SetBool("Stepping", false);
		if (m_eDirection == EDirection::Left2
			|| m_eDirection == EDirection::Right2)
		{
			pChar->ChangeState(new StateIdle());
			return;
		}

		_vector dir = XMVectorZero();
		if (pGameInstance->IsKeyDown(VK_UP))    dir += XMVectorSet(0, 0, 1, 0);
		if (pGameInstance->IsKeyDown(VK_DOWN))  dir += XMVectorSet(0, 0, -1, 0);
		if (pGameInstance->IsKeyDown(VK_LEFT))  dir += XMVectorSet(-1, 0, 0, 0);
		if (pGameInstance->IsKeyDown(VK_RIGHT)) dir += XMVectorSet(1, 0, 0, 0);

		_bool bMoving = !XMVector3Equal(dir, XMVectorZero());

		if (bMoving && !anim->CheckBool("Attacking") && !anim->CheckBool("Jump"))
		{
			pChar->ChangeState(new StateMove());
		}
		else
		{
			pChar->ChangeState(new StateIdle());
		}

		return;
	}
	pChar->GetTransform()->MoveDirection(m_vStepDir, fTimeDelta);
	pChar->GetTransform()->LookAt(pChar->Get_Target()->GetTransform()->Get_State(STATE::POSITION));*/
	if (t >= 1.f)
	{
		auto anim = pChar->Get_Animator();
		anim->SetBool("Stepping", false);

		//// 1) 2단 스텝(Left2/Right2)이면 그냥 Idle
		//if (m_eDirection == EDirection::Left2
		//	|| m_eDirection == EDirection::Right2)
		//{
		//	pChar->ChangeState(new StateIdle());
		//	return;
		//}

		_vector dir = XMVectorZero();
		// 입력 벡터 수집 (월드 기준)
		if (CGameInstance::Get_Instance()->IsKeyDown(VK_LEFT))  dir = XMVectorSet(-1, 0, 0, 0);
		if (CGameInstance::Get_Instance()->IsKeyDown(VK_RIGHT)) dir = XMVectorSet(1, 0, 0, 0);
		if (CGameInstance::Get_Instance()->IsKeyDown(VK_UP))   dir += XMVectorSet(0, 0, 1, 0);
		if (CGameInstance::Get_Instance()->IsKeyDown(VK_DOWN))  dir += XMVectorSet(0, 0, -1, 0);
		if (XMVector3Equal(dir, XMVectorZero()))
		{
			// 키 입력 없으면 Idle
			pChar->ChangeState(new StateIdle(TEXT("Idle")));
			return;
		}

		_bool moving = !XMVector3Equal(dir, XMVectorZero());
		if (moving && !anim->CheckBool("Attacking") && !anim->CheckBool("Jump"))
			pChar->ChangeState(new StateMove(TEXT("Move")));
		else
			pChar->ChangeState(new StateIdle(TEXT("Idle")));

		return;
	}

	// t < 1: 기존대로 이동 및 바라보기
	pChar->GetTransform()->MoveDirection(m_vStepDir, fTimeDelta, pChar->GetNavigation());
	if (pChar->Get_Target())
		pChar->GetTransform()->LookAtXZ(pChar->Get_Target()->GetTransform()->Get_State(STATE::POSITION));
}

void StateStep::Exit(CBaseCharacter* pChar)
{
	auto anim = pChar->Get_Animator();
	anim->SetBool("Stepping", false);
}
