#pragma once
#include "Component.h"
#include "Inspector.h"

BEGIN_NAMESPACE(Engine)

class ENGINE_DLL CPhysXCollider : public CComponent
{
protected:
    CPhysXCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CPhysXCollider(const CPhysXCollider& Prototype);
    virtual ~CPhysXCollider() = default;

public:
    virtual void Update(_float fTimeDelta);
    virtual HRESULT Initialize_Prototype(PxPhysics* pPhysx, PxMaterial* pDefaultMat);
    virtual HRESULT Initialize(void* pArg) override;
    virtual void DebugDraw();
    virtual void RenderInspector(IInspector& inspector);

public:
    void SetTrigger(_bool b) { m_bIsTrigger = b; if (m_pShape) m_pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, !b); }
    _bool IsTrigger() const { return m_bIsTrigger; }

    void SetDebugDrawEnabled(_bool enable) { m_bDebugDrawEnabled = enable; }
    _bool IsDebugDrawEnabled() const { return m_bDebugDrawEnabled; }
    void SetLocalOffset(const PxVec3& off) {
        m_vLocalOffset = off;
        if (m_pShape)
            m_pShape->setLocalPose(PxTransform(m_vLocalOffset));
    }
    PxVec3 GetLocalOffset() const { return m_vLocalOffset; }

	virtual json Serialize() override;
	virtual void Deserialize(const json& j) override;

protected:
    PxPhysics* m_pPhysics = nullptr;
    PxShape* m_pShape = nullptr;
    PxRigidActor* m_pActor = nullptr;
    PxMaterial* m_pMaterial = nullptr;
    _bool       m_bIsTrigger = false;
    _bool      m_bDebugDrawEnabled = true;
    PxVec3   m_vLocalOffset = PxVec3(0);


public:
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};
END_NAMESPACE

