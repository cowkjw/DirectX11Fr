#pragma once
#include "State.h"

BEGIN_NAMESPACE(Client)
class StateAttackDown : public IState
{
public:
	StateAttackDown() = default;
	StateAttackDown(const _wstring& stateName)
		: IState(stateName) {
	}
	~StateAttackDown() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;
private:
	_bool bIsCombo{ false }; // ÄÞº¸ ¿©ºÎ
};
END_NAMESPACE