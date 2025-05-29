#pragma once
#include "State.h"
BEGIN_NAMESPACE(Client)
class StateJump : public IState
{
public:
	StateJump() = default;
	~StateJump() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, _float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;

private:
    XMFLOAT3 m_startPos;
    XMFLOAT3 m_direction;      // 입력 기반 수평 이동 방향
    XMFLOAT3 m_velocity;       // 현재 속도
    const float GRAVITY    = -50.f;  // 중력 가속도
    const float JUMP_SPEED = 30.0f;    // 초기 점프 속도
    const  float FALL_MULTIPLIER = 8.f;
    const float HORIZ_SPEED= 25.0f;    // 수평 이동 속도
    const float GROUND_Y   = 0.0f;    // 착지 높이
};
END_NAMESPACE