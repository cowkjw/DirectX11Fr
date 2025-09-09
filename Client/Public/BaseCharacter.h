#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
BEGIN_NAMESPACE(Engine)
class CShader;
class CModel;
class CAnimator;
class CBoxCollider;
class CCapsuleCollider;
class CSphereCollider;
END_NAMESPACE

BEGIN_NAMESPACE(Client)	
static constexpr _float3 GRAVITY = { 0.0f, -9.8f, 0.0f };
static constexpr float     FRICTION = 5.0f;   // 지면 마찰 계수
static constexpr float     RESTITUTION = 0.5f; // 바닥 반사 계수
class CBaseCharacter : public CGameObject,  public ICollisionListener
{
public:
	enum class CSTATE { IDLE, MOVE, ATTACK,GUARD,JUMP, STEP,
		KNOCKDOWN,SKILL,SKILL1,SKILL2, HURT,AIR, BOUND,DOWN,DIE };
protected:
	CBaseCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBaseCharacter(const CBaseCharacter& Prototype);
	virtual ~CBaseCharacter() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_Shadow();
	
public:
	void Set_Weapon(const char* boneName, class CWeapon* pWeapon);
	class CAnimator* Get_Animator() { return m_pAnimatorCom; }
	void ChangeState(class IState* pState);
	const _wstring& GetCurrentStateName();
	class CInputBuffer* GetInputBuffer() { return m_pInputBuffer; }
	virtual void HandleInput();
	virtual void UpdateState(_float fTimeDelta);

	virtual void FillInput(InputData& outInput);

	void SetIsJumping(_bool bIsJumping) { m_bIsJumping = bIsJumping; }
	void Set_Target(const _wstring& name, LEVEL eLevel);
	void SetLastStepDirection(EDirection eDirection) { m_eLastStepDirection = eDirection; }
	void SetState(CSTATE eState) { m_eState = eState; } // 캐릭터 상태
	void SetVelocity(const _float3& velocity) {
		m_Velocity = velocity;
		if (m_Velocity.y > 0.f) // 점프 중일 때
		{
			m_bAirborne = true;
		}
	}


	_bool IsAirborne() const { return m_bAirborne; } // 공중에 떠 있는지 여부
	_bool IsJumping() const { return m_bIsJumping; }
	_bool IsFalling() const { return m_bFalling; }
	CSTATE GetState() const { return m_eState; }
	EDirection GetLastStepDirection() const { return m_eLastStepDirection; } // 마지막 방향
	const _float3& GetVelocity() const { return m_Velocity; }
	CGameObject* Get_Target() const { return m_pTarget; }
	class CNavigation* GetNavigation() const { return m_pNavigationCom; }
	class CDashSmokeEffect* GetDashSmokeEffect() const { return m_pDashSmokeEffect; }

	void LaunchAirborne(_float fJumpForce = 10.f, _bool bIsBound = false); // 에어본
	void LaunchAirborneFall(_float fJumpForce = 10.f); // 에어본
	void Blow(CGameObject* pAttacker, _float fBlowForce = 10.f); // 넉백
	void UpdateAirborne(_float fTimeDelta);
	void UpdateBounding(_float fTimeDelta);
	void PushBack(CBaseCharacter* pAttacker);
	void HurtDown();
	virtual void ActiveCollider() {};
	virtual void DeactiveCollider() {};
	virtual void TakeDamage(_float fDamage);

	virtual void OnAttackHit(CGameObject* pTarget) {};

	// 히트 스탑
	void StartHitStop(_float duration);

	void SetGameStarted(_bool bStarted) { m_bGameStarted = bStarted; }
	virtual HRESULT Ready_Effects() { return S_OK; }
protected:
	virtual void Ready_Animation();
	void SpawnGurad();

protected:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CCapsuleCollider* m_pColliderCom = { nullptr };
	CAnimator* m_pAnimatorCom = { nullptr };
	class IState* m_pState = { nullptr };
	class CNavigation* m_pNavigationCom = { nullptr }; // 네비게이션 컴포넌트
protected:
	virtual HRESULT Ready_Components();
	virtual HRESULT Bind_Shaders();

protected:
	_bool m_bFirstCollision{ false }; // 첫 충돌 여부
	_bool m_bAirborne{ false }; // 공중에 떠 있는지 여부
	_bool m_bIsJumping{ false }; // 점프 중인지 여부
	_bool m_bIsBound{ false };// 바운드 상태인지 여부
	_bool m_bCanBlowAttack{ false };
	_bool m_bCanRangeAttack{ false }; // 범위 공격 가능 여부 ex) 아카자 스킬이나 쿄주로 방어 스킬
	_bool m_bFalling{ false }; // 낙하 중인지 여부
	_bool m_bGameStarted{ false }; // 게임 시작 여부

	_float m_fMaxHP{ 100.f };           // 최대 체력
	_float m_fCurrentHP{ 100.f };       // 현재 체력
	_float m_fStamina{ 0.f };         // 스태미나(호흡력)
	_float m_fTimeDelta{ 0.f };
	_float m_fTotalTime{ 0.f }; // 총 시간 
	_float m_fGoroundHeight{ 0.f }; // 바닥 높이

	// 히트 스탑
	_float m_fHitStopTime = 0.f;  // 남은 히트스탑 시간


	CSTATE m_eState{ CSTATE::IDLE }; // 현재 상태
	CGameObject* m_pTarget{ nullptr };

	CSphereCollider* m_pRangeColliderCom{ nullptr }; // 추가 충돌체
	class CGuardEffect* m_pGuardEffect = { nullptr }; // 가드 이펙트


	class CDashSmokeEffect* m_pDashSmokeEffect = { nullptr };
	class CWeapon* m_pWeapon{ nullptr }; // 무기
	class CInputBuffer* m_pInputBuffer{ nullptr }; // 입력 버퍼 (커맨드 패턴)
	EDirection m_eLastStepDirection{ EDirection::NONE }; // 마지막 이동 방향

	_float3 m_Velocity = { 0, 0, 0 };


	_uint m_iShaderPass{ 0 }; // 셰이더 패스 인덱스
public:
	static CBaseCharacter* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;


	// ICollisionListener을(를) 통해 상속됨
	void OnCollisionEnter(CCollider* other) override;
	void OnCollisionEnter(CCollider* other, const XMFLOAT3& hitPos) override;

	void OnCollisionStay(CCollider* other, _float fTimeDelta) override;

	void OnCollisionExit(CCollider* other) override;

};
END_NAMESPACE

