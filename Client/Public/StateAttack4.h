#pragma once
#include "State.h"
BEGIN_NAMESPACE(Client)
class StateAttack4 : public IState
{
public:
	StateAttack4() = default;
	StateAttack4(const _wstring& stateName)
		: IState(stateName) {
	}
	~StateAttack4() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, _float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;
private:
	_bool bIsCombo{ false }; // ÄÞº¸ ¿©ºÎ
};
END_NAMESPACE