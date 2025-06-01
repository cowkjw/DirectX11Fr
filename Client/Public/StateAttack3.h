#pragma once
#include "State.h"
BEGIN_NAMESPACE(Client)
class StateAttack3 : public IState
{
public:
	StateAttack3() = default;
	StateAttack3(const _wstring& stateName)
		: IState(stateName) {
	}
	~StateAttack3() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, _float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;
private:
	_bool bIsCombo{ false }; // ÄÞº¸ ¿©ºÎ
};
END_NAMESPACE