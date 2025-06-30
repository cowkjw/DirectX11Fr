#pragma once
#include "BossState.h"
class BossDeath : public BossState
{
public:
	BossDeath() : BossState(L"Death") {}
	virtual ~BossDeath() = default;
	virtual void Enter(CEnmuMeat* pChar) override;
	virtual void Update(CEnmuMeat* pChar, _float fTimeDelta) override;
	virtual void Exit(CEnmuMeat* pChar) override;
private:
	_bool m_bIsOpen{ false }; // ¿ÀÇÂ ¿©ºÎ
};

