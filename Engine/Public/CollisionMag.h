#pragma once
#include "Base.h"

BEGIN_NAMESPACE(Engine)
class CCollisionMag final : public CBase
{
private:
	CCollisionMag() = default;
	virtual ~CCollisionMag() = default;

public:
	void Register(class CCollider* collider) { m_vColliders.push_back(collider); Safe_AddRef(collider); }

	void Unregister(class CCollider* collider)
	{
		auto it = find(m_vColliders.begin(), m_vColliders.end(), collider);
		if (it != m_vColliders.end())
		{
			Safe_Release(*it);
			m_vColliders.erase(it);
		}
	}

	void Update(_float fTimeDelta);
	void DebugDraw();
	void Clear();

private:
	void ResolvePenetrationXZ(class CCollider* A, class CCollider* B);

private:
	vector<class CCollider*> m_vColliders; // 충돌체 목록
	set<pair<class CCollider*, class CCollider*>> m_vCollisions;
public:
	static CCollisionMag* Create();
	virtual void Free() override;
};
END_NAMESPACE

