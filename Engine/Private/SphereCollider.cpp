#include "SphereCollider.h"

CSphereCollider::CSphereCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPhysXCollider(pDevice,pContext)
{
}

CSphereCollider::CSphereCollider(const CSphereCollider& Prototype)
    : CPhysXCollider(Prototype),
    m_fRadius(Prototype.m_fRadius)

{
}

HRESULT CSphereCollider::Initialize_Prototype(PxPhysics* pPhysx, PxMaterial* pDefaultMat, _float radius)
{
    if (FAILED(__super::Initialize_Prototype(pPhysx, pDefaultMat)))
        return E_FAIL;
    m_fRadius = radius;
    m_pShape = pPhysx->createShape(PxSphereGeometry(m_fRadius), *m_pMaterial);
    return S_OK;
}

HRESULT CSphereCollider::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
    return S_OK;
}

void CSphereCollider::RenderInspector(IInspector& inspector)
{
    if (inspector.TreeNode("Sphere Collider")) {
        _bool changed = false;
        _float r = m_fRadius;
        if (inspector.DragFloat("Radius", &r, 0.1f)) changed = true;
        _float off[3] = { m_vLocalOffset.x, m_vLocalOffset.y, m_vLocalOffset.z };
        if (inspector.DragFloat3("Offset", off, 0.1f))
        {
            m_vLocalOffset = PxVec3(off[0], off[1], off[2]);
            changed = true;
        }
        if (changed)
        {
            r = (r > 0.1f ? r : 0.1f);
            m_fRadius = r;

            PxShape* oldShape = m_pShape;
            m_pActor->detachShape(*oldShape);
            oldShape->release();
            m_pShape = m_pPhysics->createShape(PxSphereGeometry(m_fRadius), *m_pMaterial);
            m_pShape->setLocalPose(PxTransform(m_vLocalOffset));  // 바로 반영
            m_pActor->attachShape(*m_pShape);
        }
        CPhysXCollider::RenderInspector(inspector);
        inspector.TreePop();
    }
}

void CSphereCollider::DebugDraw()
{
}

CSphereCollider* CSphereCollider::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, PxPhysics* pPhysx, PxMaterial* pDefaultMat, _float radius)
{
    CSphereCollider* pInstance = new CSphereCollider(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pPhysx, pDefaultMat, radius)))
    {
        MSG_BOX("Failed to Created : CSphereCollider");
        Safe_Release(pInstance);
    }

    return pInstance;
}


CComponent* CSphereCollider::Clone(void* pArg)
{
    CSphereCollider* pInstance = new CSphereCollider(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CSphereCollider");
        Safe_Release(pInstance);
    }

    return pInstance;

}

void CSphereCollider::Free()
{
    __super::Free();
}
