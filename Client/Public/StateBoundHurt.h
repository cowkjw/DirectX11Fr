#pragma once
#include "State.h"
class StateBoundHurt :public IState
{
public:
	StateBoundHurt() : IState(TEXT("BoundHurt")) {}
	virtual ~StateBoundHurt() = default;
	virtual void Enter(CBaseCharacter* pChar) override;
	virtual void Update(CBaseCharacter* pChar, const InputData& input, _float fTimeDelta) override;
	virtual void Exit(CBaseCharacter* pChar) override;
};

