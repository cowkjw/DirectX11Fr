#pragma once
#include "State.h"
class StateHurt :public IState
{
public:
	StateHurt() : IState(TEXT("Hurt")) {}
	virtual ~StateHurt() = default;
	virtual void Enter(CBaseCharacter* pChar) override;
	virtual void Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta) override;
	virtual void Exit(CBaseCharacter* pChar) override;
};

