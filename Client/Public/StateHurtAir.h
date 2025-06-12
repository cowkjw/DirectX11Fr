#pragma once

#include "State.h"
class StateHurtAir :public IState
{
public:
	StateHurtAir() : IState(TEXT("HurtAir")) {}
	virtual ~StateHurtAir() = default;
	virtual void Enter(CBaseCharacter* pChar) override;
	virtual void Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta) override;
	virtual void Exit(CBaseCharacter* pChar) override;
};

