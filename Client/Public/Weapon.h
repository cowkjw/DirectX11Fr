#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
BEGIN_NAMESPACE(Engine)
class CShader;
class CModel;
class CRigidBody;
class CPhysXCollider;
class CBoxCollider;
class CCapsuleCollider;
class CSphereCollider;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CWeapon : public CGameObject
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
private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CCapsuleCollider* m_pColliderCom = { nullptr };

private:
	virtual HRESULT Ready_Components();

public:
	static CWeapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};
END_NAMESPACE

