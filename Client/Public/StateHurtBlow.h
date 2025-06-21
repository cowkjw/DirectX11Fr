#pragma once
#include "State.h"
class StateHurtBlow :public IState
{
public:
	StateHurtBlow() : IState(TEXT("HurtBlow")) {}
	virtual ~StateHurtBlow() = default;
	virtual void Enter(CBaseCharacter* pChar) override;
	virtual void Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta) override;
	virtual void Exit(CBaseCharacter* pChar) override;
};

