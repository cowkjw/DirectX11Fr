#pragma once
#include "BossState.h"

BEGIN_NAMESPACE(Client)
class BossFollowPunch :
    public BossState
{
public:
	BossFollowPunch() = default;
	BossFollowPunch(const _wstring& stateName)
		: BossState(stateName) {
	}
	~BossFollowPunch() override = default;
	void Enter(CEnmuMeat* pChar) override;
	void Update(CEnmuMeat* pChar, _float fTimeDelta) override;
	void Exit(CEnmuMeat* pChar) override;
	
private:
	_matrix ComputeMatrix(CEnmuMeat* pChar,_bool bIsLeft);
private:
	_float m_fTimeElapsed{ 0.f };
	_bool m_bAttackedLeft = false; // 왼팔 공격 여부
	_bool m_bAttackedRight = false; // 오른팔 공격 여부
	const _float LEFTARM_START_TIME = 3.5f; // 왼팔 공격 시작 시간
	const _float RIGHTARM_START_TIME = 3.f; // 오른팔 공격 시작 시간
	const _float ATTACK_END_TIME = 7.5f;
	class CBone* m_pLeftArmBone{ nullptr };
	class CBone* m_pRightArmBone{ nullptr };
};
END_NAMESPACE

