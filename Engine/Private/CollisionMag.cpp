#include "CollisionMag.h"
#include "Collider.h"

void CCollisionMag::Update(_float fTimeDelta)
{
	for (auto& collider : m_vColliders)
    {
		collider->Update();
	}

    set<pair<CCollider*, CCollider*>> curr;
    for (size_t i = 0; i < m_vColliders.size(); ++i)
    {
        for (size_t j = i + 1; j < m_vColliders.size(); ++j) 
        {
			auto A = m_vColliders[i], B = m_vColliders[j]; // A, B 순서대로 비교
            if (A->Intersects(B))
            {
				curr.insert({ A,B }); // 현재 충돌 상태를 저장
                if (!m_vCollisions.count({ A,B })) // 처음이면 
                {
                    A->NotifyEnter(B); 
                    B->NotifyEnter(A);
                }
				else // 이미 충돌 중이면
                {
                    A->NotifyStay(B, fTimeDelta);
                    B->NotifyStay(A, fTimeDelta);
                }
            }
        }
    }
    for (auto& p : m_vCollisions) 
    {
		if (!curr.count(p)) // 현재 충돌 상태에 없으면
        {
            p.first->NotifyExit(p.second);
            p.second->NotifyExit(p.first);
        }
    }
	m_vCollisions = move(curr); // 현재 충돌 상태로 업데이트
}

CCollisionMag* CCollisionMag::Create()
{
	return  new CCollisionMag();
}

void CCollisionMag::Free()
{
	for (auto& collider : m_vColliders) 
    {
		Safe_Release(collider);
	}
	m_vColliders.clear();
	m_vCollisions.clear();
}
