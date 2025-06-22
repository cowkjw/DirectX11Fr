#pragma once
#include "BossState.h"

BEGIN_NAMESPACE(Client)
class BossFollowPunch :public BossState
{
	struct TwoBoneIKResult
	{
		_matrix shoulderRotation;
		_matrix elbowRotation;
		_matrix wristRotation;
	};
public:
	BossFollowPunch() = default;
	BossFollowPunch(const _wstring& stateName)
		: BossState(stateName) {
	}
	~BossFollowPunch() override = default;
	void Enter(CEnmuMeat* pChar) override;
	void Update(CEnmuMeat* pChar, _float fTimeDelta) override;
	void Exit(CEnmuMeat* pChar) override;
	_matrix ComputeMatrix(CEnmuMeat* pChar, _bool bIsLeft);

private:
	void CalculateArmLengths(CEnmuMeat* pChar);
	void ApplyIK(CEnmuMeat* pChar, _bool bIsLeft, _vector vTargetPos);
	TwoBoneIKResult CalculateTwoBoneIK(_vector shoulderPos, _vector targetPos, float upperArmLength, float forearmLength);
	_matrix LookRotation(_vector direction, _vector up);

private:
	_float m_fTimeElapsed{ 0.f };
	_float m_fInitialLeftY{ 0.f };
	_float m_fInitialRightY{ 0.f };
	_bool m_bAttackedLeft = false; // ¿ÞÆÈ °ø°Ý ¿©ºÎ
	_bool m_bAttackedRight = false; // ¿À¸¥ÆÈ °ø°Ý ¿©ºÎ
	const _float LEFTARM_START_TIME = 3.5f; // ¿ÞÆÈ °ø°Ý ½ÃÀÛ ½Ã°£
	const _float RIGHTARM_START_TIME = 3.f; // ¿À¸¥ÆÈ °ø°Ý ½ÃÀÛ ½Ã°£
	const _float ATTACK_END_TIME = 7.5f;
	class CBone* m_pLeftArmBone{ nullptr };
	class CBone* m_pRightArmBone{ nullptr };

	class CBone* m_pLeftShoulderBone = nullptr;
	class CBone* m_pLeftElbowBone = nullptr;
	class CBone* m_pLeftWristBone = nullptr;

	class CBone* m_pRightShoulderBone = nullptr;
	class CBone* m_pRightElbowBone = nullptr;
	class CBone* m_pRightWristBone = nullptr;

	// ÆÈ ±æÀÌ Á¤º¸
	_float m_fLeftUpperArmLength = 0.f;
	_float m_fLeftForearmLength = 0.f;
	_float m_fRightUpperArmLength = 0.f;
	_float m_fRightForearmLength = 0.f;
	CGameObject* m_pWarningZone{ nullptr };
};
END_NAMESPACE

