#include "CapsuleCollider.h"
#include "GameObject.h"
#include "SphereCollider.h"
#include "BoxCollider.h"

CCapsuleCollider::CCapsuleCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCollider(pDevice, pContext)
{
}

CCapsuleCollider::CCapsuleCollider(const CCapsuleCollider& Prototype)
	: CCollider(Prototype),
	m_fRadius(Prototype.m_fRadius),
	m_fHalfHeight(Prototype.m_fHalfHeight)
{
}

HRESULT CCapsuleCollider::Initialize_Prototype(_float radius, _float halfHeight)
{

	m_fRadius = radius > 0.1f ? radius : 0.1f; // 최소값 제한
	m_fHalfHeight = halfHeight > 0.1f ? halfHeight : 0.1f; // 최소값 제한

	return S_OK;
}

HRESULT CCapsuleCollider::Initialize(void* pArg)
{
	
	if (m_pOwner)
	{
		auto pTransform = m_pOwner->GetTransform();
		_vector pos = pTransform->Get_State(STATE::POSITION);

		XMVECTOR center = pos + XMLoadFloat3(&m_offset);
		XMVECTOR quat = pTransform->Get_RotationQuaternion();
		// 간단화: 캡슐을 두 AABB로 근사
		XMFLOAT3 up(0, m_fHalfHeight, 0);
		XMVECTOR upv = XMLoadFloat3(&up);
		XMVECTOR a = center + upv;
		XMVECTOR b = center - upv;
		XMStoreFloat3(&CapsuleCapsuleA.Center, a);
		XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&CapsuleCapsuleA.Orientation), quat);
		CapsuleCapsuleA.Extents = XMFLOAT3(m_fRadius, m_fRadius, m_fRadius);
		XMStoreFloat3(&CapsuleCapsuleB.Center, b);
		XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&CapsuleCapsuleB.Orientation), quat);
		CapsuleCapsuleB.Extents = XMFLOAT3(m_fRadius, m_fRadius, m_fRadius);
	}
	__super::Initialize(pArg);
	return S_OK;
}

void CCapsuleCollider::Update()
{
	auto pTransform = m_pOwner->GetTransform();
	_vector pos = pTransform->Get_State(STATE::POSITION);

	XMVECTOR center = pos + XMLoadFloat3(&m_offset);
	XMVECTOR quat = pTransform->Get_RotationQuaternion();
	// 간단화: 캡슐을 두 AABB로 근사
	XMFLOAT3 up(0, m_fHalfHeight, 0);
	XMVECTOR upv = XMLoadFloat3(&up);
	XMVECTOR a = center + upv;
	XMVECTOR b = center - upv;
	XMStoreFloat3(&CapsuleCapsuleA.Center, a);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&CapsuleCapsuleA.Orientation), quat);
	CapsuleCapsuleA.Extents = XMFLOAT3(m_fRadius, m_fRadius, m_fRadius);
	XMStoreFloat3(&CapsuleCapsuleB.Center, b);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&CapsuleCapsuleB.Orientation), quat);
	CapsuleCapsuleB.Extents = XMFLOAT3(m_fRadius, m_fRadius, m_fRadius);
}


void CCapsuleCollider::RenderInspector(IInspector& inspector)
{
	if (inspector.TreeNode("Capsule Collider")) {
		_bool changed = false;
		_float r = m_fRadius;
		_float h = m_fHalfHeight;
		if (inspector.DragFloat("Radius", &r, 0.1f) || inspector.DragFloat("Half Height", &h, 0.1f)) changed = true;

		if (changed)
		{
			r = (r > 0.1f ? r : 0.1f);
			h = (h > 0.1f ? h : 0.1f);
			m_fRadius = r; m_fHalfHeight = h;

		}
		CCollider::RenderInspector(inspector);
		inspector.TreePop();
	}
}

void CCapsuleCollider::DebugDraw()
{
}

json CCapsuleCollider::Serialize()
{
	json j = CCollider::Serialize();
	j["Type"] = "CapsuleCollider";
	j["Radius"] = m_fRadius;
	j["HalfHeight"] = m_fHalfHeight;
	return j;
}

void CCapsuleCollider::Deserialize(const json& j)
{
	CCollider::Deserialize(j);
	if (j.contains("Radius") && j.contains("HalfHeight"))
	{
		auto radius = j["Radius"];
		auto halfHeight = j["HalfHeight"];
		m_fRadius = radius;
		m_fHalfHeight = halfHeight;
	}
}

_bool CCapsuleCollider::Intersects(CCollider* other)
{
	other->Update();
	auto otherCapsule = dynamic_cast<CCapsuleCollider*>(other);
	if (otherCapsule)
		return CapsuleCapsuleA.Intersects(otherCapsule->CapsuleCapsuleA) || CapsuleCapsuleB.Intersects(otherCapsule->CapsuleCapsuleB);
	auto otherSphere = dynamic_cast<CSphereCollider*>(other);
	if (otherSphere)
		return CapsuleCapsuleA.Intersects(otherSphere->GetBoundingSphere()) || CapsuleCapsuleB.Intersects(otherSphere->GetBoundingSphere());
	auto otherBox = dynamic_cast<CBoxCollider*>(other);
	if (otherBox)
		return CapsuleCapsuleA.Intersects(otherBox->GetBoundingBox()) || CapsuleCapsuleB.Intersects(otherBox->GetBoundingBox());
	return false;
}

CCapsuleCollider* CCapsuleCollider::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _float radius, _float halfHeight)
{
	CCapsuleCollider* pInstance = new CCapsuleCollider(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(radius, halfHeight)))
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
