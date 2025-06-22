#pragma once
#include "BossState.h"


BEGIN_NAMESPACE(Client)
class BossFreezeAttack : public BossState
{
public:
	BossFreezeAttack() = default;
	BossFreezeAttack(const _wstring& stateName)
		: BossState(stateName) {
	}
	~BossFreezeAttack() override = default;
	void Enter(CEnmuMeat* pChar) override;
	void Update(CEnmuMeat* pChar, _float fTimeDelta) override;
	void Exit(CEnmuMeat* pChar) override;

private:
	_float m_fTimeElapsed{ 0.f };
	_float m_fFollowTime{ 3.f };
	_float m_fDetectTime{ 2.2f };
	_float ATTACK_END_TIME{ 5.f }; // 공격이 끝나는 시간
	_bool m_bAttckArm{ false }; // ture면 오른쪽 팔 공격, false면 왼쪽 팔 공격
	_bool m_bAttacked{ false }; // 공격이 끝났는지 여부
	CGameObject* m_pWarningZone{ nullptr }; // 경고 구역 오브젝트
};
END_NAMESPACE

