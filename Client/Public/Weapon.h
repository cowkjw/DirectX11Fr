#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
BEGIN_NAMESPACE(Engine)
class CShader;
class CModel;
class CBone;
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

	void LaucnhTargetAirborne(class CBaseCharacter* pTarget, _float fForce = 10.f);
	void ClearDamagedTargets() {	m_DamagedTargets.clear();	}
private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CSphereCollider* m_pColliderCom = { nullptr };
	CSphereCollider* m_pColliderCom1 = { nullptr };
	CSphereCollider* m_pColliderCom2 = { nullptr };
	CBone* m_pBoneSocket = { nullptr };
	_float4x4 m_CombinedWorldMatrix{};
	_bool m_bFirstCollision = true; // 첫 충돌 여부

private:
	virtual HRESULT Ready_Components();
	virtual HRESULT Bind_Shaders();

private:
	unordered_set<CGameObject*> m_DamagedTargets;

public:
	static CWeapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;


	// ICollisionListener을(를) 통해 상속됨
	virtual void OnCollisionEnter(CCollider* other) override;
	virtual void OnCollisionEnter(class CCollider* other, const XMFLOAT3& hitPos);

	virtual void OnCollisionStay(CCollider* other, float fTimeDelta) override;
	virtual void OnCollisionExit(CCollider* other) override;

};
END_NAMESPACE

