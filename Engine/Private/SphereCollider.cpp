#include "SphereCollider.h"
#include "GameObject.h"
#include "BoxCollider.h"
#include "CapsuleCollider.h"

CSphereCollider::CSphereCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCollider(pDevice, pContext)
{
	m_eColliderType = ColliderType::SPHERE;
}

CSphereCollider::CSphereCollider(const CSphereCollider& Prototype)
	: CCollider(Prototype),
	m_fRadius(Prototype.m_fRadius)

{
}



HRESULT CSphereCollider::Initialize_Prototype(_float radius)
{
	m_fRadius = radius > 0.1f ? radius : 0.1f; // 최소값 제한
	return S_OK;
}

HRESULT CSphereCollider::Initialize(void* pArg)
{
	if (m_pOwner)
	{
		auto pTransform = m_pOwner->GetTransform();
		if (pTransform)
		{
			Sphere.Radius = m_fRadius;
			auto pos = pTransform->Get_State(STATE::POSITION);
			XMVECTOR center = XMVectorSetW(pos, 0.f) + XMVectorSet(m_offset.x, m_offset.y, m_offset.z, 0.f); // W를 0으로 설정하여 위치 벡터로 사용
			XMStoreFloat3(&Sphere.Center, center);

		}
		else
		{
			return E_FAIL; // Transform이 없으면 초기화 실패
		}
	}
	__super::Initialize(pArg);
	return S_OK;
}

void CSphereCollider::Update()
{
	auto pTransform = m_pOwner->GetTransform();
	if (pTransform)
	{
		// 1) 반지름 동기화
		Sphere.Radius = m_fRadius;

		// 2) 월드 매트릭스 불러오기
		XMMATRIX worldMtx = XMLoadFloat4x4(&pTransform->Get_WorldMatrix());

		// 3) 로컬 오프셋 벡터 (w=0 → 방향 벡터로 취급)
		XMVECTOR offsetV = XMVectorSet(m_offset.x, m_offset.y, m_offset.z, 0.f);

		// 4) 회전·스케일만 적용된 오프셋
		XMVECTOR rotatedOffset = XMVector3TransformNormal(offsetV, worldMtx);

		// 5) 월드 위치 + 회전된 오프셋
		XMVECTOR center = XMVectorSetW(pTransform->Get_State(STATE::POSITION), 0.f) + rotatedOffset;

		// 6) 저장
		XMStoreFloat3(&Sphere.Center, center);

	}
}

void CSphereCollider::RenderInspector(IInspector& inspector)
{
	if (inspector.TreeNode("Sphere Collider"))
	{
		_bool changed = false;
		_float radius = m_fRadius;
		if (inspector.DragFloat("Radius", &radius, 0.1f))
		{
			m_fRadius = radius > 0.1f ? radius : 0.1f; // 최소값 제한
			changed = true;
		}
		if (changed)
		{
			Sphere.Radius = m_fRadius;
			Sphere.Center = m_offset; // Offset 적용
		}
		CCollider::RenderInspector(inspector);
		inspector.TreePop();
	}
}

void CSphereCollider::DebugDraw()
{
	if (!m_bIsDebugDraw)
		return;
	CCollider::DebugDraw();
	m_pEffect->Apply(m_pContext);
	m_pBatch->Begin();
	Draw(m_pBatch, Sphere, m_bIsCollision ? Colors::Red : Colors::Green);
	m_pBatch->End();
}

json CSphereCollider::Serialize()
{
	json j = CCollider::Serialize();
	j["Type"] = "SphereCollider";
	j["Radius"] = m_fRadius;
	return j;
}

void CSphereCollider::Deserialize(const json& j)
{
	CCollider::Deserialize(j);
	if (j.contains("Radius"))
	{
		auto radius = j["Radius"];
		m_fRadius = radius;
	}
}

_bool CSphereCollider::Intersects(CCollider* other)
{
	other->Update();
	auto otherSphere = dynamic_cast<CSphereCollider*>(other);
	if (otherSphere)
	{
		return Sphere.Intersects(otherSphere->Sphere);
	}

	if (auto box = dynamic_cast<CBoxCollider*>(other))
	{
		return Sphere.Intersects(box->GetBoundingBox());
	}

	if (auto capsule = dynamic_cast<CCapsuleCollider*>(other))
	{
		return capsule->Intersects(this); // 캡슐과의 충돌 검사
	}
	return false;
}

CSphereCollider* CSphereCollider::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _float radius)
{
	CSphereCollider* pInstance = new CSphereCollider(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(radius)))
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
