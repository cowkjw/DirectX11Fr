#pragma once
#include "BaseCharacter.h"

BEGIN_NAMESPACE(Client)
class CAkaza :public CBaseCharacter
{
public:
	enum class COM_STATE
	{
		IDLE,
		MOVE,
		ATTACK,
		GUARD,
		STEP,
		SKILL,
		JUMP,
		APPRACH,
		HURT,
		DIE,
	};
private:
	CAkaza(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAkaza(const CAkaza& Prototype);
	virtual ~CAkaza() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void SetComState(COM_STATE eComState) { m_eComState = eComState; }
	COM_STATE GetComState() const { return m_eComState; }
	virtual void TakeDamage(_float fDamage) override;
	virtual void OnAttackHit(CGameObject* pTarget) override;
	virtual void OnCollisionEnter(CCollider* other) override;
private:
	virtual HRESULT Ready_Components() override;
	virtual void Ready_Animation() override;

	_bool IsCooldownReady(_float& fCooldownVar, _float fTimeDelta, _float fBaseDelay);
	virtual void FillInput(InputData& outInput) override;
	virtual void HandleInput() override;

	void ActiveCollider();
	void DeactiveCollider();


private:
	
	COM_STATE m_eComState = COM_STATE::IDLE;
	_float m_fFollowTime = 5.f; // 플레이어를 따라가는 시간
	_float m_fAttackCooldown = 0.f;   // 평타 재사용 대기(예: 0.5초)
	_float m_fSkill0Cooldown = 0.f;   // 스킬0 재사용 대기
	_float m_fSkill1Cooldown = 0.f;   // 스킬1 재사용 대기
	_float m_fSkill2Cooldown = 0.f;   // 스킬2 재사용 대기
	_float m_fFollowCooldown = 0.f; // 플레이어 추적 재사용 대기
	_float m_fStepCooldown = 0.f;     // 스텝 회피 재사용 대기
	_float m_fGuardCooldown = 0.f;     // 스텝 회피 재사용 대기
	_float m_fNextDecisionTime = 0.f; // 다음 행동 결정 시간
	_float m_fJumpCooldown = 0.f; // 점프 대기 시간
	int   m_comboStep = 0;
	float m_comboCooldown = 0.f;   // 다음 콤보 공격을 위한 쿨다운
	// 공격 콤보 간격 (초)
	static constexpr float COMBO_DELAY = 0.3f;
	_vector m_lastMoveDir = XMVectorZero();
	mt19937 m_RandGen;
	uniform_real_distribution<_float> m_Distribution{ 0.f, 1.f };

public:
	static CAkaza* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

