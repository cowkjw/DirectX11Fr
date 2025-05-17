#pragma once
#include "PhysXCollider.h"

BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CBoxCollider final : public CPhysXCollider
{
private:
	CBoxCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBoxCollider(const CBoxCollider& Prototype);
	virtual ~CBoxCollider() = default;
public:
	HRESULT Initialize_Prototype(PxPhysics* pPhysx, PxMaterial* pDefaultMat, const PxVec3& extents);
	void RenderInspector(IInspector& inspector) override;
	void DebugDraw() override;
	json Serialize() override;
	void Deserialize(const json& j) override;
private:
	PxVec3 m_vHalfExtents = PxVec3(0.5f);

public:
	static CBoxCollider* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,PxPhysics* pPhysx, PxMaterial* pDefaultMat, const PxVec3& halfExtents);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

