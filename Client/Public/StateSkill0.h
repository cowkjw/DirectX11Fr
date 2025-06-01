#pragma once
#include "State.h"

BEGIN_NAMESPACE(Client)
class StateSkill0 : public IState
{
public:
	StateSkill0() = default;
	StateSkill0(const _wstring& stateName)
		: IState(stateName) {
	}
	~StateSkill0() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;
};
END_NAMESPACE