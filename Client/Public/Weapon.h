#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
BEGIN_NAMESPACE(Engine)
class CShader;
class CModel;
class CBone;
class CRigidBody;
class CPhysXCollider;
class CBoxCollider;
class CCapsuleCollider;
class CSphereCollider;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CWeapon : public CGameObject,public ICollisionListener
{
protected:
	CWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWeapon(const CWeapon& Prototype);
	virtual ~CWeapon() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
	
public:
	void Set_BoneSocket(CBone* pBoneSocket) {
		m_pBoneSocket = pBoneSocket;
	}

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CSphereCollider* m_pColliderCom = { nullptr };
	CSphereCollider* m_pColliderCom1 = { nullptr };
	CSphereCollider* m_pColliderCom2 = { nullptr };
	CBone* m_pBoneSocket = { nullptr };
	_float4x4 m_OffsetMatrix{};

private:
	virtual HRESULT Ready_Components();
	virtual HRESULT Bind_Shaders();

public:
	static CWeapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;


	// ICollisionListener을(를) 통해 상속됨
	void OnCollisionEnter(CCollider* other) override;

	void OnCollisionStay(CCollider* other, float fTimeDelta) override;

	void OnCollisionExit(CCollider* other) override;

};
END_NAMESPACE

