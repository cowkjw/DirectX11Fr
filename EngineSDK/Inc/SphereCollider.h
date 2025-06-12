#pragma once
#include "Collider.h"

BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CSphereCollider final : public CCollider
{
	friend class CCollisionMag;
private:
	CSphereCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSphereCollider(const CSphereCollider& Prototype);
	virtual ~CSphereCollider() = default;
public:
	HRESULT Initialize_Prototype(_float radius);
	HRESULT Initialize(void* pArg) override;
	virtual void Update() override;
	void RenderInspector(IInspector& inspector) override;
	void DebugDraw() override;

	json Serialize() override;
	void Deserialize(const json& j) override;
	// CCollider을(를) 통해 상속됨
	_bool Intersects(CCollider* other) override;
	const BoundingSphere& GetBoundingSphere() const {
		return Sphere;
	}
	void SetRadius(_float radius) {
		m_fRadius = radius;
		Sphere.Radius = radius;
	}
private:
	BoundingSphere Sphere;
	_float m_fRadius = 0.5f;

public:
	static CSphereCollider* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,_float radius);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

