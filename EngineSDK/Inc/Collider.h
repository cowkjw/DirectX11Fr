#pragma once
#include "Component.h"
#include "Inspector.h"
BEGIN_NAMESPACE(Engine)



class ENGINE_DLL CCollider : public CComponent
{
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

	void SetListener(ICollisionListener* listener) { m_listener = listener; }
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
	_float3   m_offset{};
	_bool       m_bIsTrigger;
	ICollisionListener* m_listener = nullptr;

public:
	virtual void Free() override;
};
END_NAMESPACE
