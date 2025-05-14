#include "CapsuleCollider.h"

CCapsuleCollider::CCapsuleCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPhysXCollider(pDevice,pContext)
{
}

CCapsuleCollider::CCapsuleCollider(const CCapsuleCollider& Prototype)
    : CPhysXCollider(Prototype),
    m_fRadius(Prototype.m_fRadius),   
    m_fHalfHeight(Prototype.m_fHalfHeight)
{
}

HRESULT CCapsuleCollider::Initialize_Prototype(PxPhysics* pPhysx, PxMaterial* pDefaultMat, _float radius, _float halfHeight)
{
    if (FAILED(__super::Initialize_Prototype(pPhysx, pDefaultMat)))
        return E_FAIL;
    m_fRadius = radius;
    m_fHalfHeight = halfHeight;
    m_pShape = m_pPhysics->createShape(PxCapsuleGeometry(m_fRadius, m_fHalfHeight), *m_pMaterial);
    return S_OK;
}

void CCapsuleCollider::RenderInspector(IInspector& inspector)
{
    if (inspector.TreeNode("Capsule Collider")) {
        _bool changed = false;
        _float r = m_fRadius;
        _float h = m_fHalfHeight;
        if (inspector.DragFloat("Radius", &r, 0.1f) || inspector.DragFloat("Half Height", &h, 0.1f)) changed = true;
        _float off[3] = { m_vLocalOffset.x, m_vLocalOffset.y, m_vLocalOffset.z };
        if (inspector.DragFloat3("Offset", off, 0.1f))
        {
            m_vLocalOffset = PxVec3(off[0], off[1], off[2]);
            changed = true;
        }
        if (changed)
        {
            r = (r > 0.1f ? r : 0.1f);
            h = (h > 0.1f ? h : 0.1f);
            m_fRadius = r; m_fHalfHeight = h;
            PxShape* oldShape = m_pShape;
            m_pActor->detachShape(*oldShape);
            oldShape->release();
            m_pShape = m_pPhysics->createShape(PxCapsuleGeometry(m_fRadius, m_fHalfHeight), *m_pMaterial);
            m_pShape->setLocalPose(PxTransform(m_vLocalOffset));  // 바로 반영
            m_pActor->attachShape(*m_pShape);
        }
        CPhysXCollider::RenderInspector(inspector);
        inspector.TreePop();
    }
}

void CCapsuleCollider::DebugDraw()
{
}

CCapsuleCollider* CCapsuleCollider::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, PxPhysics* pPhysx, PxMaterial* pDefaultMat, _float radius, _float halfHeight)
{
    CCapsuleCollider* pInstance = new CCapsuleCollider(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pPhysx, pDefaultMat, radius, halfHeight)))
    {
        MSG_BOX("Failed to Created : CCapsuleCollider");
        Safe_Release(pInstance);
    }

    return pInstance;
}


CComponent* CCapsuleCollider::Clone(void* pArg)
{
    CCapsuleCollider* pInstance = new CCapsuleCollider(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CCapsuleCollider");
        Safe_Release(pInstance);
    }

    return pInstance;

}

void CCapsuleCollider::Free()
{
    __super::Free();
}
