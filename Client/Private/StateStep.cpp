#include "StateStep.h"
#include "StateIdle.h"
#include "StateMove.h"
#include "DashSmokeEffect.h"
#include <Tanjiro.h>
#include "GameInstance.h"


void StateStep::Enter(CBaseCharacter* pChar)
{

	auto pAnim = pChar->Get_Animator();
	m_fElapsedTime = 0.f;
	pAnim->SetBool("Move", true);
	pAnim->SetBool("Stepping", true);

	if (pChar->Get_Name() == TEXT("Kyojuro"))
	{
		CSoundMag::Get_Instance()->PlayEffect("event:/Kyojuro/Step");
	}
	else if (pChar->Get_Name() == TEXT("Akaza"))
	{
		CSoundMag::Get_Instance()->PlayEffect("event:/Akaza/Step");
	}
	else
	{
		CSoundMag::Get_Instance()->PlayEffect("event:/Tanjiro/Step");
	}

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

	m_fElapsedTime += fTimeDelta;
	_float t = m_fElapsedTime / m_fStepDuration;
	if (t >= 1.f)
	{
		auto anim = pChar->Get_Animator();
		anim->SetBool("Stepping", false);

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

	// t < 1 기존대로 이동 및 바라보기
	pChar->GetTransform()->MoveDirection(m_vStepDir, fTimeDelta, pChar->GetNavigation());
	if (pChar->Get_Target())
		pChar->GetTransform()->LookAtXZ(pChar->Get_Target()->GetTransform()->Get_State(STATE::POSITION));
}

void StateStep::Exit(CBaseCharacter* pChar)
{
	auto anim = pChar->Get_Animator();
	anim->SetBool("Stepping", false);
}
