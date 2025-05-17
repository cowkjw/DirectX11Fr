#pragma once
#include "Component.h"
#include "Inspector.h"

BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CRigidBody final  :public CComponent
{
private:
    CRigidBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CRigidBody(const CRigidBody& Prototype);
    ~CRigidBody() = default;

public:

    HRESULT Initialize_Prototype(PxPhysics* pPhysx, PxMaterial* pMat, const PxTransform& transform);
    virtual HRESULT Initialize(void* pArg) override;

    PxRigidActor* GetRigidActor() const { return m_pDynamic; }
    void SetKinematic(_bool bIsKinematic)
    {
        m_bIsKinematic = bIsKinematic;
        if (m_pDynamic)
            m_pDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, m_bIsKinematic);
    }

    void RenderInspector(IInspector& inspector);
	virtual json Serialize() override;
	virtual void Deserialize(const json& j) override;
	void SetMass(_float fMass) { m_fMass = fMass; }
	_float GetMass() const { return m_fMass; }
	void SetUseGravity(_bool bUseGravity) { m_bUseGravity = bUseGravity; }
	_bool GetUseGravity() const { return m_bUseGravity; }

private:
    PxRigidDynamic* m_pDynamic = nullptr;   // 물리 엔진 RigidDynamic 객체
    PxPhysics* m_pPhysics = nullptr;   // PhysX Physics 객체
    PxMaterial* m_pMaterial = nullptr;   // 재질 정보
    _float              m_fMass = 1.0f;      // 질량
    _bool               m_bUseGravity = true;      // 중력 적용 여부
    _bool               m_bIsKinematic = false;     // 키네틱(비물리 제어) 모드 여부
public:
    static CRigidBody* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, PxPhysics* pPhysx, PxMaterial* pMat, const PxTransform& transform);
    virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE 

