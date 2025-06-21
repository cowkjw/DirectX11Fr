#pragma once
#include "State.h"
class StateHurtDown :public IState
{
public:
	StateHurtDown() : IState(TEXT("HurtDown")) {}
	virtual ~StateHurtDown() = default;
	virtual void Enter(CBaseCharacter* pChar) override;
	virtual void Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta) override;
	virtual void Exit(CBaseCharacter* pChar) override;
};

