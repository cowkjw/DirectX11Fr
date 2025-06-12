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
private:
	_bool m_bIsLeftArm = false; // 왼팔인지 오른팔인지 구분
	class CBodyColliderParts* m_pBodyColliderCom{nullptr};

	_float m_fDefaultRadius = 15.f; // 콜라이더의 반지름

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

