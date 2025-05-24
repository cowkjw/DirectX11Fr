#pragma once
#include "State.h"

BEGIN_NAMESPACE(Client)
class StateAttackDown : public IState
{
public:
	StateAttackDown() = default;
	~StateAttackDown() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, _float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;
private:
	_bool bIsCombo{ false }; // ÄÞº¸ ¿©ºÎ
};
END_NAMESPACE