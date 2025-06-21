#pragma once
#include "EnmuParts.h"

BEGIN_NAMESPACE(Client)
class CEnmuArm final :  public CEnmuParts
{
public:
	typedef struct ArmDesc : public GAMEOBJECT_DESC
	{
		_wstring 	sModelKey;
	} ARM_DESC;
private:
	CEnmuArm(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnmuArm(const CEnmuArm& Prototype);
	virtual ~CEnmuArm() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	virtual HRESULT Ready_Components() override;
	void Ready_AnimationForLeft() ;
	void Ready_AnimationForRight() ;
	virtual void Ready_Collider() override;

	void ActiveCollider() override;
	void DeactiveCollider() override;
	void SetCollisionRadius(_float fRadius);
	void StartRotateY(_float duration, _float targetYawDeg);
private:
	_bool m_bIsLeftArm = false; // 왼팔인지 오른팔인지 구분

	_bool   m_bRotating = false;   // 보간 중 플래그
	_float  m_fRotateTimer = 0.f;     // 경과 시간
	_float  m_fRotateDur = 0.5f;    // 회전 지속 시간
	_float  m_fStartYaw = 0.f;     // 시작 Y각
	_float  m_fTargetYaw = 0.f;     // 목표 Y각
	class CBodyColliderParts* m_pBodyColliderCom{nullptr};

	_float m_fDefaultRadius = 30.f; // 콜라이더의 반지름
	

public:
	static CEnmuArm* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,void* pArg = nullptr);
	static CGameObject* Clone(void* pArg);
	virtual void Free() override;


	// ICollisionListener을(를) 통해 상속됨
	virtual void OnCollisionEnter(CCollider* other) override;
	virtual void OnCollisionStay(CCollider* other, float fTimeDelta) override;
	virtual void OnCollisionExit(CCollider* other) override;

};
END_NAMESPACE

