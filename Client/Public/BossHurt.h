#pragma once
#include "BossState.h"

BEGIN_NAMESPACE(Client)
class BossHurt : public BossState
{
public:
	BossHurt() = default;
	BossHurt(const _wstring& stateName)
		: BossState(stateName) {
	}
	~BossHurt() override = default;
	void Enter(CEnmuMeat* pChar) override;
	void Update(CEnmuMeat* pChar, _float fTimeDelta) override;
	void Exit(CEnmuMeat* pChar) override;
};
END_NAMESPACE

