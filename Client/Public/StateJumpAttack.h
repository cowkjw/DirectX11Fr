#pragma once
#include "State.h"
BEGIN_NAMESPACE(Client)
class StateJumpAttack : public IState
{
public:
	StateJumpAttack() = default;
	StateJumpAttack(const _wstring& stateName)
		: IState(stateName) {
	}
	~StateJumpAttack() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;
};
END_NAMESPACE