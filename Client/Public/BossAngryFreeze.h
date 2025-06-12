#pragma once
#include "BossState.h"

BEGIN_NAMESPACE(Client)
class BossAngryFreeze : public BossState
{
public:
	BossAngryFreeze() = default;
	BossAngryFreeze(const _wstring& stateName)
		: BossState(stateName) {
	}
	~BossAngryFreeze() override = default;
	void Enter(CEnmuMeat* pChar) override;
	void Update(CEnmuMeat* pChar, _float fTimeDelta) override;
	void Exit(CEnmuMeat* pChar) override;

private:
	_float m_fTimeElapsed{ 0.f }; // 경직 시간 누적
	const _float ATTACK_END_TIME = 6.f;
	_int m_iKeyPressCount{ 0 }; // 키 입력 횟수
};
END_NAMESPACE

