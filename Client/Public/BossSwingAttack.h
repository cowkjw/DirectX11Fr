#pragma once
#include "BossState.h"
BEGIN_NAMESPACE(Client)
class BossSwingAttack : public BossState
{
public:
	BossSwingAttack() = default;
	BossSwingAttack(const _wstring& stateName)
		: BossState(stateName) {
	}
	~BossSwingAttack() override = default;
	void Enter(CEnmuMeat* pChar) override;
	void Update(CEnmuMeat* pChar, _float fTimeDelta) override;
	void Exit(CEnmuMeat* pChar) override;
private:
	_float m_fTimeElapsed = 0.f; // 경과 시간
	_bool m_bAttacked = false;
	const _float CHARGING_TIME = 4.5f; // 차징하는 시간
	const _float ATTACK_END_TIME = 6.f;
	CGameObject* pWarning = nullptr;
};
END_NAMESPACE

