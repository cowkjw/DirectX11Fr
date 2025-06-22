#pragma once
#include "State.h"
class StateDeath : public IState
{
public:
	StateDeath() : IState(L"Death") {}
	virtual ~StateDeath() = default;
	virtual void Enter(CBaseCharacter* pChar) override;
	virtual void Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta) override;
	virtual void Exit(CBaseCharacter* pChar) override;
};

