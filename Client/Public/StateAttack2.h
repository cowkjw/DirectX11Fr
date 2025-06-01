#pragma once
#include "State.h"
BEGIN_NAMESPACE(Client)
class StateAttack2 : public IState
{
public:
	StateAttack2() = default;
	StateAttack2(const _wstring& stateName)
		: IState(stateName) {
	}
	~StateAttack2() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, _float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;
private:
	_bool bIsCombo{ false }; // ÄÞº¸ ¿©ºÎ
};
END_NAMESPACE