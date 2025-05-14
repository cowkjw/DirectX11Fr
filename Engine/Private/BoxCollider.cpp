#include "BoxCollider.h"

CBoxCollider::CBoxCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPhysXCollider(pDevice,pContext)
{
}

CBoxCollider::CBoxCollider(const CBoxCollider& Prototype)
    : CPhysXCollider(Prototype),
    m_vHalfExtents(Prototype.m_vHalfExtents)

{
}

HRESULT CBoxCollider::Initialize_Prototype(PxPhysics* pPhysx, PxMaterial* pDefaultMat, const PxVec3& extents)
{
    if (FAILED(__super::Initialize_Prototype(pPhysx, pDefaultMat)))
        return E_FAIL;
    m_vHalfExtents = extents;
	m_pPhysics = pPhysx;
	m_pMaterial = pDefaultMat;
    m_pShape = pPhysx->createShape(PxBoxGeometry(m_vHalfExtents), *m_pMaterial);
    m_pShape->setFlag(PxShapeFlag::eVISUALIZATION, true);
    return S_OK;
}

void CBoxCollider::RenderInspector(IInspector& inspector)
{
    if (inspector.TreeNode("Box Collider")) {
        _float ext[3] = { m_vHalfExtents.x, m_vHalfExtents.y, m_vHalfExtents.z };
        _bool changed = false;
        // Drag controls
        if (inspector.DragFloat3("Half Extents", ext, 0.1f)) changed = true;

        _float off[3] = { m_vLocalOffset.x, m_vLocalOffset.y, m_vLocalOffset.z };
        if (inspector.DragFloat3("Offset", off, 0.1f))
        {
            m_vLocalOffset = PxVec3(off[0], off[1], off[2]);
            changed = true;
        }
        if (changed) {
            for (int i = 0; i < 3; ++i)
                ext[i] = (ext[i] > 0.1f ? ext[i] : 0.1f);
            if (m_pShape && m_pActor)
                m_pActor->detachShape(*m_pShape);
            m_pShape->release();

            m_vHalfExtents = PxVec3(ext[0], ext[1], ext[2]);
            m_pShape = m_pPhysics->createShape(PxBoxGeometry(m_vHalfExtents), *m_pMaterial);
            m_pShape->setLocalPose(PxTransform(m_vLocalOffset));  // 바로 반영
            m_pActor->attachShape(*m_pShape);
        }
        CPhysXCollider::RenderInspector(inspector);
        inspector.TreePop();
    }
}

void CBoxCollider::DebugDraw()
{
}

CBoxCollider* CBoxCollider::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, PxPhysics* pPhysx, PxMaterial* pDefaultMat, const PxVec3& halfExtents)
{
    CBoxCollider* pInstance = new CBoxCollider(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pPhysx, pDefaultMat, halfExtents)))
    {
        MSG_BOX("Failed to Created : CBoxCollider");
        Safe_Release(pInstance);
    }

    return pInstance;
}


CComponent* CBoxCollider::Clone(void* pArg)
{
    CBoxCollider* pInstance = new CBoxCollider(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CBoxCollider");
        Safe_Release(pInstance);
    }

    return pInstance;

}

void CBoxCollider::Free()
{
    __super::Free();
}
