#pragma once
#include "State.h"

BEGIN_NAMESPACE(Client)
class StateGuard : public IState
{
public:
	StateGuard() = default;
	StateGuard(const _wstring& stateName)
		: IState(stateName) {
	}
	~StateGuard() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;
};
END_NAMESPACE