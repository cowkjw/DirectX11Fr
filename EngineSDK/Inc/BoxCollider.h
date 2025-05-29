#pragma once
#include "Collider.h"

BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CBoxCollider final : public CCollider
{
private:
	CBoxCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBoxCollider(const CBoxCollider& Prototype);
	virtual ~CBoxCollider() = default;
public:
	virtual HRESULT Initialize_Prototype(const _float3 vHalfExtents);
	virtual HRESULT Initialize(void* pArg);
	virtual void Update();
	virtual void DebugDraw();
	virtual void RenderInspector(IInspector& inspector);
	virtual json Serialize()override;
	virtual void Deserialize(const json& j) override;

	// CCollider을(를) 통해 상속됨
	_bool Intersects(CCollider* other) override;
	const BoundingOrientedBox& GetBoundingBox() const {
		return Box;
	}
private:
	BoundingOrientedBox Box;
	_float3 m_vHalfExtents{ 1.f,1.f,1.f };

public:
	static CBoxCollider* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,const _float3& vHalfExtents = _float3(1.f,1.f,1.f));
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};
END_NAMESPACE

