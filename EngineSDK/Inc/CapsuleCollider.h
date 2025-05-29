#pragma once
#include "Collider.h"

BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CCapsuleCollider final : public CCollider
{
private:
	CCapsuleCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCapsuleCollider(const CCapsuleCollider& Prototype);
	virtual ~CCapsuleCollider() = default;
public:
	HRESULT Initialize_Prototype(_float radius, _float halfHeight);
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update();
	void RenderInspector(IInspector& inspector) override;
	void DebugDraw() override;

	json Serialize() override;
	void Deserialize(const json& j) override;
	_bool Intersects(CCollider* other) override;
	const BoundingOrientedBox& GetBoundingCapsuleA() const {
		return CapsuleCapsuleA;
	}
	const BoundingOrientedBox& GetBoundingCapsuleB() const {
		return CapsuleCapsuleB;
	}
private:
	_float m_fRadius = 0.5f;
	_float m_fHalfHeight = 1.0f;
	BoundingOrientedBox CapsuleCapsuleA;
	BoundingOrientedBox CapsuleCapsuleB;

public:
	static CCapsuleCollider* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,_float radius = 0.5f, _float halfHeight = 1.f);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

