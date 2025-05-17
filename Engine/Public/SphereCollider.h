#pragma once
#include "PhysXCollider.h"

BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CSphereCollider final : public CPhysXCollider
{
private:
	CSphereCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSphereCollider(const CSphereCollider& Prototype);
	virtual ~CSphereCollider() = default;
public:
	HRESULT Initialize_Prototype(PxPhysics* pPhysx, PxMaterial* pDefaultMat, _float radius);
	HRESULT Initialize(void* pArg) override;
	void RenderInspector(IInspector& inspector) override;
	void DebugDraw() override;

	json Serialize() override;
	void Deserialize(const json& j) override;
private:
	_float m_fRadius = 0.5f;

public:
	static CSphereCollider* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,PxPhysics* pPhysx, PxMaterial* pDefaultMat,_float radius);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

