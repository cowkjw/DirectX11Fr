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
	enum class CSTATE { IDLE, MOVE, ATTACK, GUARD,JUMP, STEP, KNOCKDOWN,SKILL, HURT, DIE };
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
	_bool IsJumping() const { return m_bIsJumping; }
	void Set_Target(const _wstring& name, LEVEL eLevel);
	CGameObject* Get_Target() const { return m_pTarget; }
	void SetLastStepDirection(EDirection eDirection) { m_eLastStepDirection = eDirection; }

	void SetState(CSTATE eState) { m_eState = eState; } // 캐릭터 상태
	CSTATE GetState() const { return m_eState; }

	EDirection GetLastStepDirection() const { return m_eLastStepDirection; } // 마지막 방향
	const _float3& GetVelocity() const { return m_Velocity; }
	void SetVelocity(const _float3& velocity) { m_Velocity = velocity; }

	void LaunchAirborne(_float fJumpForce = 10.f);
	void UpdateAirborne(_float fTimeDelta);
	virtual void ActiveCollider() {};
	virtual void DeactiveCollider() {};
	virtual void TakeDamage(_float fDamage) {
		m_fCurrentHP -= fDamage;
		if (m_fCurrentHP <= 0.f)
		{
			m_fCurrentHP = 0.f;
		}
	}

	class CNavigation* GetNavigation() const { return m_pNavigationCom; }
protected:
	virtual void Ready_Animation();

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
	_float m_fMaxHP{ 0.f };           // 최대 체력
	_float m_fCurrentHP{ 0.f };       // 현재 체력
	_float m_fStamina{ 0.f };         // 스태미나(호흡력)
	_float m_fTimeDelta{ 0.f };
	CSTATE m_eState{ CSTATE::IDLE }; // 현재 상태
	class CWeapon* m_pWeapon{ nullptr }; // 무기
	CGameObject* m_pTarget{ nullptr };

	class CInputBuffer* m_pInputBuffer{ nullptr }; // 입력 버퍼 (커맨드 패턴)
	_float m_fTotalTime{ 0.f }; // 총 시간 
	EDirection m_eLastStepDirection{ EDirection::NONE }; // 마지막 이동 방향

	_float3 m_Velocity = { 0, 0, 0 };
	_uint m_iShaderPass{ 0 }; // 셰이더 패스 인덱스

public:
	static CBaseCharacter* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;


	// ICollisionListener을(를) 통해 상속됨
	void OnCollisionEnter(CCollider* other) override;

	void OnCollisionStay(CCollider* other, _float fTimeDelta) override;

	void OnCollisionExit(CCollider* other) override;

};
END_NAMESPACE

