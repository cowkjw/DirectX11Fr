#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
#include "EnmuMeat.h"	
BEGIN_NAMESPACE(Engine)
class CShader;
class CModel;
class CAnimator;
class CBoxCollider;
class CCapsuleCollider;
class CSphereCollider;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CEnmuParts :  public CGameObject, public ICollisionListener
{
protected:
	CEnmuParts(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnmuParts(const CEnmuParts& Prototype);
	virtual ~CEnmuParts() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void ChangeState(EnmuState eState) { m_eState = eState; }
	class CAnimator* Get_Animator() const { return m_pAnimatorCom; }
	virtual void ActiveCollider() {};
	virtual void DeactiveCollider() {};
protected:
	virtual HRESULT Ready_Components();
	virtual void Ready_Collider();
	virtual void Ready_Animation();
	virtual HRESULT Bind_Shaders();


protected:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CAnimator*  m_pAnimatorCom	= { nullptr };
	CSphereCollider* m_pColliderCom = { nullptr };
	_float m_fMaxHP = 100.f;
	_float m_fCurrentHP = 100.f;
	_uint m_iShaderPass{ 0 }; // 셰이더 패스 인덱스
	_wstring m_strModelKey; // 모델 키
	EnmuState m_eState = EnmuState::IDLE; // 현재 상태 
public:
	virtual void Free() override;
	virtual CGameObject* Clone(void* pArg) override { return new CEnmuParts(*this); };
	// ICollisionListener을(를) 통해 상속됨
	virtual void OnCollisionEnter(CCollider* other) override;
	virtual void OnCollisionStay(CCollider* other, float fTimeDelta) override;
	virtual void OnCollisionExit(CCollider* other) override;
};
END_NAMESPACE

