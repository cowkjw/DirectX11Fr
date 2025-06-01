#include "BoxCollider.h"
#include "GameObject.h"
#include <SphereCollider.h>
#include "CapsuleCollider.h"

CBoxCollider::CBoxCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCollider(pDevice, pContext)
	, m_vHalfExtents(1.f, 1.f, 1.f) // 기본값 설정
{
	m_eColliderType = ColliderType::BOX; // ColliderType 설정
}

CBoxCollider::CBoxCollider(const CBoxCollider& Prototype)
	: CCollider(Prototype)
	, m_vHalfExtents(Prototype.m_vHalfExtents) 
	, Box(Prototype.Box) // BoundingBox 복사
{

}

HRESULT CBoxCollider::Initialize_Prototype(const _float3 vHalfExtents)
{
	m_vHalfExtents = vHalfExtents;
	return S_OK;
}

HRESULT CBoxCollider::Initialize(void* pArg)
{
	if (m_pOwner)
	{
		auto pTransform = m_pOwner->GetTransform();
		if (pTransform)
		{
			_vector pos = pTransform->Get_State(STATE::POSITION);
			XMVECTOR center = XMVectorSetW(pos, 0.f); // W를 0으로 설정하여 위치 벡터로 사용
			XMVECTOR quat = pTransform->Get_RotationQuaternion();
			XMStoreFloat3(&Box.Center, center);
			Box.Extents = m_vHalfExtents;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&Box.Orientation), quat);
		}
		else
		{
			return E_FAIL; // Transform이 없으면 초기화 실패
		}
	}
	__super::Initialize(pArg);
	return S_OK;
}

void CBoxCollider::Update()
{
	if (m_pOwner)
	{
		auto pTransform = m_pOwner->GetTransform();
		if (pTransform)
		{
		
			_vector pos = pTransform->Get_State(STATE::POSITION);
			XMVECTOR center = pos + XMLoadFloat3(&m_offset);
			XMVECTOR quat = pTransform->Get_RotationQuaternion();
			XMStoreFloat3(&Box.Center, center);
			Box.Extents = m_vHalfExtents;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&Box.Orientation), quat);
		}
	}
}

void CBoxCollider::RenderInspector(IInspector& inspector)
{
	if (inspector.TreeNode("Box Collider"))
	{
		_bool changed = false;
		_float3 halfExtents = { m_vHalfExtents.x, m_vHalfExtents.y, m_vHalfExtents.z };
		if (inspector.DragFloat3("Half Extents", &halfExtents.x, 0.1f))
		{
			m_vHalfExtents = halfExtents;
			changed = true;
		}
		CCollider::RenderInspector(inspector);
		inspector.TreePop();
	}
}

_bool CBoxCollider::Intersects(CCollider* other)
{
	other->Update();
	if (auto box = dynamic_cast<CBoxCollider*>(other))
	{
		return Box.Intersects(box->GetBoundingBox());
	}
	else if (auto sphere = dynamic_cast<CSphereCollider*>(other))
	{
		return Box.Intersects(sphere->GetBoundingSphere());
	}
	else if (auto capsule = dynamic_cast<CCapsuleCollider*>(other))
	{
		return capsule->Intersects(this);
	}
	return false;
}

void CBoxCollider::DebugDraw()
{

	if (!m_bIsDebugDraw)
		return;
	if (!m_pBatch || !m_pEffect || !m_pInputLayout)
		return;
	CCollider::DebugDraw();
	m_pEffect->Apply(m_pContext);
	m_pBatch->Begin();

	Draw(m_pBatch, Box, m_bIsCollision ? Colors::Red : Colors::Green);
	m_pBatch->End();

}

json CBoxCollider::Serialize()
{
	json j = CCollider::Serialize();
	j["Type"] = "BoxCollider";
	j["HalfExtents"] = { m_vHalfExtents.x, m_vHalfExtents.y, m_vHalfExtents.z };

	return j;
}

void CBoxCollider::Deserialize(const json& j)
{
	CCollider::Deserialize(j);
	if (j.contains("HalfExtents"))
	{
		auto halfExtents = j["HalfExtents"];
		if (halfExtents.is_array() && halfExtents.size() == 3)
		{
			m_vHalfExtents.x = halfExtents[0].get<_float>();
			m_vHalfExtents.y = halfExtents[1].get<_float>();
			m_vHalfExtents.z = halfExtents[2].get<_float>();
		}
	}
}

CBoxCollider* CBoxCollider::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,const _float3& vHalfExtents)
{
	CBoxCollider* pInstance = new CBoxCollider(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(vHalfExtents)))
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
