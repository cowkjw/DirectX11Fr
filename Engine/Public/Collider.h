#pragma once
#include "Component.h"
#include "Inspector.h"
BEGIN_NAMESPACE(Engine)



class ENGINE_DLL CCollider : public CComponent
{
	friend class CCollisionMag;
public:
	enum class ColliderType
	{
		BOX,
		SPHERE,
		CAPSULE,
		HITBOX,
		HURTBOX,
		NONE
	};
protected:
	CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCollider(const CCollider& Prototype);
	virtual ~CCollider() = default;
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Update();
	virtual void DebugDraw();
	virtual void RenderInspector(IInspector& inspector);
	virtual json Serialize()override;
	virtual void Deserialize(const json& j) override;
public:
	virtual _bool Intersects(CCollider* other) = 0;
	void SetTrigger(_bool bIsTrigger) { m_bIsTrigger = bIsTrigger; }
	void SetDrawDebug(_bool bIsDebugDraw) { m_bIsDebugDraw = bIsDebugDraw; }
	void SetCollision(_bool bIsCollision) { m_bIsCollision = bIsCollision; }
	void SetOffset(const _float3& offset) { m_offset = offset; }
	void SetListener(ICollisionListener* listener) { m_listener = listener; }
	void SetColliderType(ColliderType type) { m_eColliderType = type; }

	void SetPriority(_int iPriority) { m_iPriority = iPriority; }
	_int GetPriority() const { return m_iPriority; }

	ColliderType GetType() const { return m_eColliderType; }
	void NotifyEnter(CCollider* other) {
		if (m_listener) m_listener->OnCollisionEnter(other);
	}
	void NotifyStay(CCollider* other, _float fTimeDelta) {
		if (m_listener) m_listener->OnCollisionStay(other, fTimeDelta);
	}
	void NotifyExit(CCollider* other) {
		if (m_listener) m_listener->OnCollisionExit(other);
	}

protected:
	ColliderType m_eColliderType{ ColliderType::NONE };
	_bool       m_bIsTrigger{ false };
	_bool       m_bIsCollision{ false };
	_bool 	    m_bIsDebugDraw{ true };
	_int        m_iPriority{ 0 }; // 충돌 우선순위 (높을수록 우선)
	_float3   m_offset{};
	ICollisionListener* m_listener = nullptr;
	PrimitiveBatch<VertexPositionColor>* m_pBatch = nullptr;
	BasicEffect* m_pEffect = nullptr;
	ID3D11InputLayout* m_pInputLayout = nullptr;


public:
	virtual void Free() override;
};
END_NAMESPACE
