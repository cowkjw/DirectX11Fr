#include "RigidBody.h"
#include "GameInstance.h"

CRigidBody::CRigidBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent(pDevice, pContext)
{
}

CRigidBody::CRigidBody(const CRigidBody& Prototype)
    : CComponent(Prototype),
    m_pDynamic(Prototype.m_pDynamic),
    m_pPhysics(Prototype.m_pPhysics),
    m_pMaterial(Prototype.m_pMaterial)

{
}

HRESULT CRigidBody::Initialize_Prototype(PxPhysics* pPhysx, PxMaterial* pMat, const PxTransform& transform)
{
    m_pPhysics = pPhysx;
    m_pMaterial = pMat;
    m_pDynamic = pPhysx->createRigidDynamic(transform);
    m_pDynamic->setMass(m_fMass);
    // 중력 비활성화 플래그 설정
    m_pDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !m_bUseGravity);
    // 키네틱 플래그 설정
    m_pDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, m_bIsKinematic);

	CGameInstance::Get_Instance()->GetScene()->addActor(*m_pDynamic);
    return S_OK;
}

HRESULT CRigidBody::Initialize(void* pArg)
{
    return S_OK;
}

void CRigidBody::RenderInspector(IInspector& inspector)
{
    if (inspector.TreeNode("RigidBody")) {
        // 키네틱 모드
        _bool kin = m_bIsKinematic;
        if (inspector.Checkbox("Is Kinematic", &kin)) {
            SetKinematic(kin);
        }
        // 중력
        _bool g = m_bUseGravity;
        if (inspector.Checkbox("Use Gravity", &g)) {
            m_bUseGravity = g;
            if (m_pDynamic)
                m_pDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !m_bUseGravity);
        }
        // 질량
        float mass = m_fMass;
        if (inspector.DragFloat("Mass", &mass,0.5f)) {
            m_fMass = mass;
            if (m_pDynamic)
                m_pDynamic->setMass(m_fMass);
        }
        inspector.TreePop();
    }
}

json CRigidBody::Serialize()
{
	
	json j = CComponent::Serialize();
	j["isKinematic"] = m_bIsKinematic;
	j["useGravity"] = m_bUseGravity;
	j["mass"] = m_fMass;
	return j;
}

void CRigidBody::Deserialize(const json& j)
{
	CComponent::Deserialize(j);
	if (j.contains("isKinematic"))
		m_bIsKinematic = j["isKinematic"].get<_bool>();
	if (j.contains("useGravity"))
		m_bUseGravity = j["useGravity"].get<_bool>();
	if (j.contains("mass"))
		m_fMass = j["mass"].get<_float>();
	if (m_pDynamic)
	{
		m_pDynamic->setMass(m_fMass);
		m_pDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !m_bUseGravity);
	}
}

CRigidBody* CRigidBody::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, PxPhysics* pPhysx, PxMaterial* pMat, const PxTransform& transform)
{
    CRigidBody* pInstance = new CRigidBody(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pPhysx, pMat, transform)))
    {
        MSG_BOX("Failed to Created : CRigidBody");
        Safe_Release(pInstance);
    }

    return pInstance;
}


CComponent* CRigidBody::Clone(void* pArg)
{
    CRigidBody* pInstance = new CRigidBody(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CRigidBody");
        Safe_Release(pInstance);
    }

    return pInstance;

}

void CRigidBody::Free()
{
    __super::Free();
}
