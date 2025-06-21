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
class CBodyColliderParts : public CGameObject, public ICollisionListener
{
public:
	typedef struct tagBodyColliderPartsDesc : public CGameObject::GAMEOBJECT_DESC
	{
		vector<_float3> vColliderOffsets; // 각 콜라이더의 오프셋
		_float fRadius = 1.5f; // 콜라이더의 반지름
		ColliderType eDefaultShape = ColliderType::SPHERE;
		ColliderType eDefaultType = ColliderType::HITBOX;
		
	} BODYCOLLIDERPARTS_DESC;
protected:
	CBodyColliderParts(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBodyColliderParts(const CBodyColliderParts& Prototype);
	virtual ~CBodyColliderParts() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
	virtual void OnEnable() override;
	virtual void OnDisable() override;

public:
	void Set_BoneSocket(CBone* pBoneSocket) {
		m_pBoneSocket = pBoneSocket;
	}
	CSphereCollider* GetCollider(_uint iIndex) const {
		if (iIndex < m_pColliderComs.size())
			return m_pColliderComs[iIndex];
		return nullptr;
	}

	void Set_Radius(_uint iIndex, _float fRadius);
	

private:
	vector<CSphereCollider*> m_pColliderComs;
	CBone* m_pBoneSocket = { nullptr };
	_float4x4 m_OffsetMatrix{};

private:
	virtual HRESULT Ready_Components();

private:
	unordered_set<CGameObject*> m_DamagedTargets;
	ColliderType m_eDefaultType = ColliderType::HITBOX; // 기본 콜라이더 타입

public:
	static CBodyColliderParts* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;


	// ICollisionListener을(를) 통해 상속됨
	void OnCollisionEnter(CCollider* other) override;

	void OnCollisionStay(CCollider* other, float fTimeDelta) override;

	void OnCollisionExit(CCollider* other) override;

};
END_NAMESPACE
