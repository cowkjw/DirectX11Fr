#pragma once
#include "State.h"

BEGIN_NAMESPACE(Client)
class StateAttackUp : public IState
{
public:
	StateAttackUp() = default;
	StateAttackUp(const _wstring& stateName)
		: IState(stateName) {
	}
	~StateAttackUp() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, _float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;
private:
	_bool bIsCombo{ false }; // ÄÞº¸ ¿©ºÎ
};
END_NAMESPACE