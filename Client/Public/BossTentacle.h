#pragma once
#include "BossState.h"

BEGIN_NAMESPACE(Client)
class BossTentacle : public BossState
{
public:
	BossTentacle() = default;
	BossTentacle(const _wstring& stateName)
		: BossState(stateName) {
	}
	~BossTentacle() override = default;
	void Enter(CEnmuMeat* pChar) override;
	void Update(CEnmuMeat* pChar, _float fTimeDelta) override;
	void Exit(CEnmuMeat* pChar) override;

private:
	_float m_fTimeElapsed = 0.f; // 경과 시간
	_float m_fAttackTimeElapsed = 0.f; // 공격 시간 경과
	_float m_fWaringTimeElapsed = 0.f; // 경고 시간 경과
	_bool m_bAttackPrepared = false; // 공격 준비 상태
	_float m_fNextAttackTime = 1.5f;
	_uint m_iTentacleIndex = 0; // 소환된 촉수 개수
	const _float ATTACK_END_TIME = 20.f;
	const _float WARING_TIME = 1.5f; // 경고 시간
};
END_NAMESPACE

