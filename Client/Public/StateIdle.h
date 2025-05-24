#pragma once
#include "State.h"
BEGIN_NAMESPACE(Client)
class StateIdle : public IState
{
public:
	StateIdle() = default;
	~StateIdle() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, _float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;
};
END_NAMESPACE

