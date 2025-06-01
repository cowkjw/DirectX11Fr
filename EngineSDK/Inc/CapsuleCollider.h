#pragma once
#include "Collider.h"

BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CCapsuleCollider final : public CCollider
{
	friend class CCollisionMag;
private:
	CCapsuleCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCapsuleCollider(const CCapsuleCollider& Prototype);
	virtual ~CCapsuleCollider() = default;
public:
	HRESULT Initialize_Prototype(_float radius, _float halfHeight);
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update();
	void RenderInspector(IInspector& inspector) override;
	void DebugDraw() override;

	json Serialize() override;
	void Deserialize(const json& j) override;
	_bool Intersects(CCollider* other) override;
private:
	_bool IntersectSegmentOBB(XMVECTOR A, XMVECTOR B, const BoundingOrientedBox& box);
private:
	Capsule m_Capsule{};
	_float   m_fHalfHeight{ 1.f }; // 캡슐의 반높이
	_float   m_fRadius{ 0.5f }; // 캡슐의 반지름

	// 점-세그먼트 / 세그먼트-세그먼트 거리 계산 (제곱)
	static float DistanceSq_PointToSegment(DirectX::XMVECTOR P, DirectX::XMVECTOR A, DirectX::XMVECTOR B);
	static float DistanceSq_SegmentToSegment(DirectX::XMVECTOR P1, DirectX::XMVECTOR Q1, DirectX::XMVECTOR P2, DirectX::XMVECTOR Q2);

public:
	static CCapsuleCollider* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,_float radius = 0.5f, _float halfHeight = 1.f);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

