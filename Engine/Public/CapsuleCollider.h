#pragma once
#include "PhysXCollider.h"

BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CCapsuleCollider final : public CPhysXCollider
{
private:
	CCapsuleCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCapsuleCollider(const CCapsuleCollider& Prototype);
	virtual ~CCapsuleCollider() = default;
public:
	HRESULT Initialize_Prototype(PxPhysics* pPhysx, PxMaterial* pDefaultMat, _float radius, _float halfHeight);
	void RenderInspector(IInspector& inspector) override;
	void DebugDraw() override;
private:
	_float m_fRadius = 0.5f;
	_float m_fHalfHeight = 1.0f;

public:
	static CCapsuleCollider* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, PxPhysics* pPhysx, PxMaterial* pDefaultMat, _float radius, _float halfHeight);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

