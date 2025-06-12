#pragma once
#include "BossState.h"
BEGIN_NAMESPACE(Client)
class BossIdle : public BossState
{
public:
	BossIdle() = default;
	BossIdle(const _wstring& stateName)
		: BossState(stateName) {
	}
	virtual ~BossIdle() = default;
	// BossState을(를) 통해 상속됨
	void Enter(CEnmuMeat* pChar) override;
	void Update(CEnmuMeat* pChar, _float fTimeDelta) override;
	void Exit(CEnmuMeat* pChar) override;

private:
	_float m_fTimeElapsed = 0.f; // 경과 시간
	_bool m_bPatternUsed[6] = {};
};
END_NAMESPACE

