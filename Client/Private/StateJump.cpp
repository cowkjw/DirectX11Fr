#include "StateJump.h"
#include "StateMove.h"
#include "StateJumpAttack.h"
#include "StateIdle.h"
#include "StateStep.h"


void StateJump::Enter(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Jump", true);
	pChar->SetIsJumping(true);


	XMStoreFloat3(&m_startPos, pChar->GetTransform()->Get_State(STATE::POSITION));

	// 3) 입력 기반 수평 방향 계산
	XMVECTOR dir = XMVectorZero();
	auto gi = CGameInstance::Get_Instance();
	if (gi->IsKeyDown(VK_UP))    dir += XMVectorSet(0, 0, 1, 0);
	if (gi->IsKeyDown(VK_DOWN))  dir += XMVectorSet(0, 0, -1, 0);
	if (gi->IsKeyDown(VK_LEFT))  dir += XMVectorSet(-1, 0, 0, 0);
	if (gi->IsKeyDown(VK_RIGHT)) dir += XMVectorSet(1, 0, 0, 0);
	dir = XMVector3Normalize(dir);
	XMStoreFloat3(&m_direction, dir);

	// 4) 초기 속도 세팅
	m_velocity = {
		XMVector3Equal(dir, XMVectorZero()) ? 0.f : m_direction.x * HORIZ_SPEED,
		JUMP_SPEED,
		XMVector3Equal(dir, XMVectorZero()) ? 0.f : m_direction.z * HORIZ_SPEED
	};

	pChar->SetState(CBaseCharacter::CSTATE::JUMP);
}

void StateJump::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto pAnim = pChar->Get_Animator();
	auto buf = pChar->GetInputBuffer();

	if (buf->CheckCommand(ECommand::LightAttack) && !pAnim->CheckBool("Attacking"))
	{
		pAnim->SetTrigger("JumpAttack");
		pAnim->SetBool("Attacking", true);
		//	pChar->ChangeState(new StateJumpAttack());
		return;
	}
	auto animCtrl = pAnim->GetAnimController();
	const string& stateName = animCtrl->GetCurrentState()->stateName;

	if (m_velocity.y < 0.0f) {
		// 하강 중이면 중력을 더 세게 적용
		m_velocity.y += GRAVITY * FALL_MULTIPLIER * fTimeDelta;
	}
	else {
		// 상승 중엔 기본 중력
		m_velocity.y += GRAVITY * 0.7f * fTimeDelta;
	}

	// 3) 위치 갱신
	XMVECTOR pos = pChar->GetTransform()->Get_State(STATE::POSITION);
	pos += XMVectorSet(m_velocity.x * fTimeDelta,
		m_velocity.y * fTimeDelta,
		m_velocity.z * fTimeDelta, 0);

	float newY = XMVectorGetY(pos);
	if (newY <= GROUND_Y)
	{
		// 땅에 닿았으니 위치를 GROUND_Y에 고정
		pos = XMVectorSetY(pos, GROUND_Y);

		// 수직 속도 초기화 (아래로 계속 누적되는 걸 방지)
		m_velocity.y = 0.0f;

		pChar->SetIsJumping(false);
		// 점프 상태 종료 플래그가 있으면 꺼줍니다.
		_bool moving = gi->IsKeyDown(VK_UP) || gi->IsKeyDown(VK_DOWN) ||
			gi->IsKeyDown(VK_LEFT) || gi->IsKeyDown(VK_RIGHT);
		if (moving)
		{
			pChar->ChangeState(new StateMove(TEXT("Move")));
			return;
		}
		else
		{
			pChar->ChangeState(new StateIdle(TEXT("Idle")));
			return;
		}
	}
	pChar->GetTransform()->Set_State(STATE::POSITION, pos);
	_vector dir = XMVectorZero();
	if (gi->IsKeyDown(VK_UP))
	{
		dir += XMVectorSet(0, 0, 1, 0);

	}
	if (gi->IsKeyDown(VK_DOWN))
	{
		dir += XMVectorSet(0, 0, -1, 0);

	}
	if (gi->IsKeyDown(VK_LEFT))
	{
		dir += XMVectorSet(-1, 0, 0, 0);

	}
	if (gi->IsKeyDown(VK_RIGHT))
	{
		dir += XMVectorSet(1, 0, 0, 0);
	}

	_bool bMoving = !XMVector3Equal(dir, XMVectorZero());

	auto pTarget = pChar->Get_Target();
	if (bMoving && gi->IsKeyDown('L') && pTarget)
	{
		EDirection stepDir = EDirection::NONE; // 기본값

		// 플레이어 적 전방 벡터 (월드 기준, 길이 1)
		_vector playerPos = pChar->GetTransform()->Get_State(STATE::POSITION);
		_vector enemyPos = pTarget->GetTransform()->Get_State(STATE::POSITION);
		_vector forwardEnemy = XMVector3Normalize(enemyPos - playerPos);

		//전방과 직교하는 우 방향 벡터 계산
		_vector up = XMVectorSet(0, 1, 0, 0);
		_vector rightEnemy = XMVector3Normalize(XMVector3Cross(up, forwardEnemy));


		//    전방 성분, 우측 성분 구하기
		dir = XMVector3Normalize(dir);  // 입력 방향 정규화
		_float fDot = XMVectorGetX(XMVector3Dot(forwardEnemy, dir));  // +면 앞, 면 뒤-
		_float rDot = XMVectorGetX(XMVector3Dot(rightEnemy, dir));  // +면 우,  면 좌-


		const _float dead = 0.3f;

		if (fabsf(fDot) > fabsf(rDot))
		{

			if (fDot > dead)
				stepDir = EDirection::Forward;
			else if (fDot < -dead)
				stepDir = EDirection::Backward;
			else
				return;  // 입력 작아서 스텝 없음
		}
		else
		{

			if (rDot > dead)
			{
				stepDir = EDirection::Right;

			}
			else if (rDot < -dead)
			{
				stepDir = EDirection::Left;
			}
			else
				return;
		}

		pChar->ChangeState(new StateStep(TEXT("Step"), stepDir));
		return;
	}

	if (stateName.find("Jump3") != string::npos)
	{

	}

}

void StateJump::Exit(CBaseCharacter* pChar)
{
	pChar->Get_Animator()->SetBool("Jump", false);
	pChar->Get_Animator()->SetBool("Attacking", false);
	pChar->SetIsJumping(false);
}
