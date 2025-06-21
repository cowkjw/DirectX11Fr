#pragma once
#include "BossState.h"
BEGIN_NAMESPACE(Client)
class BossHandAttack :   public BossState
{
public:
	BossHandAttack() = default;
	BossHandAttack(const _wstring& stateName)
		: BossState(stateName) {
	}
	~BossHandAttack() override = default;
	void Enter(CEnmuMeat* pChar) override;
	void Update(CEnmuMeat* pChar, _float fTimeDelta) override;
	void Exit(CEnmuMeat* pChar) override;
private:
	_float m_fTimeElapsed = 0.f;
	_bool m_bAttackedLeft = false; // 왼팔 공격 여부
	_bool m_bAttackedRight = false; // 오른팔 공격 여부
	const _float LEFTARM_START_TIME = 3.5f; // 왼팔 공격 시작 시간
	const _float RIGHTARM_START_TIME = 2.5f; // 오른팔 공격 시작 시간
	const _float ATTACK_END_TIME = 5.f;
	array<CGameObject*, 2> m_pWarnings;
};
END_NAMESPACE

