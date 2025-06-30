#include "CapsuleCollider.h"
#include "GameObject.h"
#include "SphereCollider.h"
#include "BoxCollider.h"

CCapsuleCollider::CCapsuleCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCollider(pDevice, pContext)
	, m_fRadius(0.5f)
	, m_fHalfHeight(1.f)

{
	m_eColliderType = ColliderType::CAPSULE;
}

CCapsuleCollider::CCapsuleCollider(const CCapsuleCollider& Prototype)
	: CCollider(Prototype),
	m_fRadius(Prototype.m_fRadius),
	m_fHalfHeight(Prototype.m_fHalfHeight)
{
}

HRESULT CCapsuleCollider::Initialize_Prototype(_float radius, _float halfHeight)
{

	m_fRadius = max(radius, 0.1f);
	m_fHalfHeight = max(halfHeight, 0.1f);
	return S_OK;
}

HRESULT CCapsuleCollider::Initialize(void* pArg)
{
	
	__super::Initialize(pArg);
	return S_OK;
}

void CCapsuleCollider::Update()
{
	auto pTransform = m_pOwner->GetTransform();
	XMVECTOR pos = pTransform->Get_State(STATE::POSITION);
	XMVECTOR upDir = pTransform->Get_State(STATE::UP);

	XMVECTOR center = pos + XMLoadFloat3(&m_offset);
	XMVECTOR halfH = upDir * m_fHalfHeight;

	m_Capsule.A = center + halfH;
	m_Capsule.B = center - halfH;
	m_Capsule.Radius = m_fRadius;
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
	if (!m_bIsDebugDraw)
		return;
	if (!m_pBatch || !m_pEffect || !m_pInputLayout)
		return;
	CCollider::DebugDraw();
	m_pEffect->Apply(m_pContext);
	m_pBatch->Begin();

	m_pBatch->DrawLine(
		VertexPositionColor(m_Capsule.A, m_bIsCollision ? Colors::Red : Colors::Green),
		VertexPositionColor(m_Capsule.B, m_bIsCollision ? Colors::Red : Colors::Green)
	);

	// 2) 끝단 원 그리기 (라인 스트립)
	const int slices = 16;
	XMVECTOR upDir = XMVector3Normalize(m_Capsule.A - m_Capsule.B);
	// 기준 벡터 선택 (upDir와 평행한 경우 회피)
	static const XMVECTOR upVec = XMVectorSet(0, 1, 0, 0);
	XMVECTOR basis = fabsf(XMVectorGetX(XMVector3Dot(upDir, upVec))) > 0.9f
		? XMVectorSet(1, 0, 0, 0)
		: upVec;
	XMVECTOR axisX = XMVector3Normalize(XMVector3Cross(upDir, basis));
	XMVECTOR axisY = XMVector3Normalize(XMVector3Cross(upDir, axisX));

	std::vector<VertexPositionColor> ringA, ringB;
	ringA.reserve(slices + 1);
	ringB.reserve(slices + 1);
	for (int i = 0; i <= slices; ++i)
	{
		float theta = XM_2PI * i / slices;
		XMVECTOR offset = axisX * (cosf(theta) * m_Capsule.Radius)
			+ axisY * (sinf(theta) * m_Capsule.Radius);
		ringA.emplace_back(m_Capsule.A + offset, m_bIsCollision ? Colors::Red : Colors::Green);
		ringB.emplace_back(m_Capsule.B + offset, m_bIsCollision ? Colors::Red : Colors::Green);
	}
	m_pBatch->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, ringA.data(), (UINT)ringA.size());
	m_pBatch->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, ringB.data(), (UINT)ringB.size());

	// 3) 연결선 그리기
	for (int i = 0; i <= slices; ++i)
	{
		m_pBatch->DrawLine(ringA[i], ringB[i]);
	}
	m_pBatch->End();

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
	if (auto cap = dynamic_cast<CCapsuleCollider*>(other))
	{
		float dist2 = DistanceSq_SegmentToSegment(m_Capsule.A, m_Capsule.B,
			cap->m_Capsule.A, cap->m_Capsule.B);
		float rsum = m_Capsule.Radius + cap->m_Capsule.Radius;
		return dist2 <= (rsum * rsum);
	}

	// 캡슐 vs 구
	if (auto sp = dynamic_cast<CSphereCollider*>(other))
	{
		XMVECTOR centerS = XMLoadFloat3(&sp->GetBoundingSphere().Center);
		float   rS = sp->GetBoundingSphere().Radius;
		float   d2 = DistanceSq_PointToSegment(centerS,
			m_Capsule.A, m_Capsule.B);
		float   rsum = m_Capsule.Radius + rS;
		return d2 <= (rsum * rsum);
	}

	if (auto bc = dynamic_cast<CBoxCollider*>(other))
	{
		BoundingOrientedBox box = bc->GetBoundingBox();
		return IntersectSegmentOBB(m_Capsule.A, m_Capsule.B, box);
	}

	return false;
}

_bool CCapsuleCollider::IntersectSegmentOBB(XMVECTOR A, XMVECTOR B, const BoundingOrientedBox& box)
{
	// OBB 로컬 공간으로 변환
	XMVECTOR c = XMLoadFloat3(&box.Center); // OBB 중심
	XMVECTOR q = XMLoadFloat4(&box.Orientation); // OBB 회전 쿼터니언
	XMMATRIX R = XMMatrixRotationQuaternion(q); // 회전 행렬
	XMMATRIX invR = XMMatrixTranspose(R); // 역행렬

	XMVECTOR localA = XMVector3Transform(A - c, invR); // A의 로컬 좌표
	XMVECTOR localB = XMVector3Transform(B - c, invR); // B의 로컬 좌표
	XMVECTOR d = localB - localA; // 방향 벡터

	// AABB extents
	_float ex = box.Extents.x; // OBB의 x축 반길이
	_float ey = box.Extents.y; // OBB의 y축 반길이
	_float ez = box.Extents.z; // OBB의 z축 반길이

	_float tMin = 0.0f, tMax = 1.0f;
	auto TestAxis = [&](_float origin, _float dir, _float minB, _float maxB)
		{
			if (fabsf(dir) < 1e-6f) {
				// 평행
				if (origin < minB || origin > maxB) return false;
			}
			else {
				_float ood = 1.0f / dir;
				_float t1 = (minB - origin) * ood;
				_float t2 = (maxB - origin) * ood;
				if (t1 > t2) std::swap(t1, t2);
				tMin = max(tMin, t1);
				tMax = min(tMax, t2);
				if (tMin > tMax) return false;
			}
			return true;
		};

	_float ox = XMVectorGetX(localA);
	_float oy = XMVectorGetY(localA);
	_float oz = XMVectorGetZ(localA);
	_float dx = XMVectorGetX(d);
	_float dy = XMVectorGetY(d);
	_float dz = XMVectorGetZ(d);

	if (!TestAxis(ox, dx, -ex, ex)) return false;
	if (!TestAxis(oy, dy, -ey, ey)) return false;
	if (!TestAxis(oz, dz, -ez, ez)) return false;

	return true;
}


float CCapsuleCollider::DistanceSq_PointToSegment(DirectX::XMVECTOR P, DirectX::XMVECTOR A, DirectX::XMVECTOR B)
{
	XMVECTOR AB = B - A;
	XMVECTOR AP = P - A;
	float ab2 = XMVectorGetX(XMVector3Dot(AB, AB));
	if (ab2 < 1e-6f)
		return XMVectorGetX(XMVector3LengthSq(P - A));

	float t = XMVectorGetX(XMVector3Dot(AP, AB)) / ab2;
	t = max(0.f, min(1.f, t)); // Clamp t to [0, 1]
	XMVECTOR closest = A + AB * t;
	return XMVectorGetX(XMVector3LengthSq(P - closest));
}

float CCapsuleCollider::DistanceSq_SegmentToSegment(DirectX::XMVECTOR P1, DirectX::XMVECTOR Q1, DirectX::XMVECTOR P2, DirectX::XMVECTOR Q2)
{
	XMVECTOR u = Q1 - P1;
	XMVECTOR v = Q2 - P2;
	XMVECTOR w = P1 - P2;

	float a = XMVectorGetX(XMVector3Dot(u, u));
	float b = XMVectorGetX(XMVector3Dot(u, v));
	float c = XMVectorGetX(XMVector3Dot(v, v));
	float d = XMVectorGetX(XMVector3Dot(u, w));
	float e = XMVectorGetX(XMVector3Dot(v, w));
	float D = a * c - b * b;
	float sN = 0, sD = D;
	float tN = 0, tD = D;

	if (D < 1e-6f) { sN = 0.0f; sD = 1.0f; tN = e; tD = c; }
	else {
		sN = (b * e - c * d);
		tN = (a * e - b * d);
		if (sN < 0) { sN = 0;    tN = e;  tD = c; }
		else if (sN > sD) { sN = sD;  tN = e + b; tD = c; }
	}

	if (tN < 0) {
		tN = 0;
		if (-d < 0) { sN = 0; }
		else if (-d > a) { sN = sD; }
		else { sN = -d; sD = a; }
	}
	else if (tN > tD) {
		tN = tD;
		if ((-d + b) < 0) { sN = 0; }
		else if ((-d + b) > a) { sN = sD; }
		else { sN = (-d + b); sD = a; }
	}

	float sc = (fabsf(sN) < 1e-6f ? 0 : sN / sD);
	float tc = (fabsf(tN) < 1e-6f ? 0 : tN / tD);

	XMVECTOR dP = w + (u * sc) - (v * tc);
	return XMVectorGetX(XMVector3LengthSq(dP));
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
