#pragma once
#include "State.h"
BEGIN_NAMESPACE(Client)
class StateSkill2 : public IState
{
public:
	StateSkill2() = default;
	~StateSkill2() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, _float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;
};
END_NAMESPACE