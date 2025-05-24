#pragma once
#include "State.h"

BEGIN_NAMESPACE(Client)
class StateMove : public IState
{
public:
	StateMove() = default;
	~StateMove() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, _float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;
};
END_NAMESPACE