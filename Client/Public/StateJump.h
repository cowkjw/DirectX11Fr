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
};
END_NAMESPACE