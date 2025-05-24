#pragma once
#include "State.h"

BEGIN_NAMESPACE(Client)
class StateSkill1 : public IState
{
public:
	StateSkill1() = default;
	~StateSkill1() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, _float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;
};
END_NAMESPACE