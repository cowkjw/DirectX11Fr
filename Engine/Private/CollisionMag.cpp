#include "CollisionMag.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"
#include  "GameObject.h"

void CCollisionMag::Update(_float fTimeDelta)
{
	//if (m_vColliders.empty()) return; // 충돌체가 없으면 업데이트하지 않음
	//for (auto& collider : m_vColliders)
 //   {
	//	collider->Update();
	//}

 //   set<pair<CCollider*, CCollider*>> curr;
 //   for (size_t i = 0; i < m_vColliders.size(); ++i)
 //   {
 //       for (size_t j = i + 1; j < m_vColliders.size(); ++j) 
 //       {
	//		auto A = m_vColliders[i], B = m_vColliders[j]; // A, B 순서대로 비교

 //           if (!A->IsActive() || !B->IsActive()) continue;

	//		if (m_vColliders[i]->GetOwner() == m_vColliders[j]->GetOwner()
 //               || m_vColliders[i]->GetOwner()->GetParent()==m_vColliders[j]->GetOwner()
 //               || m_vColliders[j]->GetOwner()->GetParent() == m_vColliders[i]->GetOwner()) // 같은 오브젝트는 비교하지 않음
	//			continue;
 //           if (A->Intersects(B))
 //           {
	//			curr.insert({ A,B }); // 현재 충돌 상태를 저장
 //               CCollider* pusher = A;
 //               CCollider* pushed = B;
 //               if (!m_vCollisions.count({ A,B })) // 처음이면 
 //               {
 //                   A->NotifyEnter(B);
 //                   B->NotifyEnter(A);
 //                   A->SetCollision(true); // 충돌 상태 설정
 //                   B->SetCollision(true);

 //                   if (A->GetPriority() > B->GetPriority())
 //                   {
 //                       pusher = A;
 //                       pushed = B;
 //                   }
 //                   else if (B->GetPriority() > A->GetPriority())
 //                   {
 //                       pusher = B;
 //                       pushed = A;
 //                   }

 //                   if (pusher->GetType() == CCollider::ColliderType::HITBOX
 //                       || pushed->GetType() == CCollider::ColliderType::HITBOX)
 //                       continue;
	//				ResolvePenetrationXZ(pusher, pushed); // 충돌 해소
 //               }
	//			else // 이미 충돌 중이면
 //               {
 //                   A->NotifyStay(B, fTimeDelta);
 //                   B->NotifyStay(A, fTimeDelta);
 //                   A->SetCollision(true); // 충돌 상태 설정
 //                   B->SetCollision(true);
 //                   if (A->GetPriority() > B->GetPriority())
 //                   {
 //                       pusher = A;
 //                       pushed = B;
 //                   }
 //                   else if (B->GetPriority() > A->GetPriority())
 //                   {
 //                       pusher = B;
 //                       pushed = A;
	//				}
 //                   if (pusher->GetType() == CCollider::ColliderType::HITBOX
 //                       || pushed->GetType() == CCollider::ColliderType::HITBOX)
 //                       continue;
 //                   ResolvePenetrationXZ(pusher, pushed); // 충돌 해소
 //               }
 //           }
 //       }
 //   }
 //   for (auto& p : m_vCollisions) 
 //   {
	//	if (!curr.count(p)) // 현재 충돌 상태에 없으면
 //       {
 //           p.first->NotifyExit(p.second);
 //           p.second->NotifyExit(p.first);
 //           p.first->SetCollision(false); // 충돌 상태 설정
 //           p.second->SetCollision(false);
 //       }
 //   }
	//m_vCollisions = move(curr); // 현재 충돌 상태로 업데이트

    if (m_vColliders.empty())
        return;

    // 1) 모든 콜라이더 업데이트
    for (auto& collider : m_vColliders)
        collider->Update();

    // 2) 이번 프레임에 충돌이 발생한 (소유자)쌍을 기록할 set
    //    pair<CBaseCharacter*, CBaseCharacter*> 형태로 저장
    set<pair<CGameObject*, CGameObject*>> ownerCollided;

    // 3) 현재 프레임의 콜라이더 간 충돌을 기록할 set
    set<pair<CCollider*, CCollider*>> currColliders;

    // 4) 모든 콜라이더 쌍에 대해 충돌 검사
    for (size_t i = 0; i < m_vColliders.size(); ++i)
    {
        auto A = m_vColliders[i];
        for (size_t j = i + 1; j < m_vColliders.size(); ++j)
        {
            auto B = m_vColliders[j];

            // (a) 둘 중 하나라도 비활성화 상태면 무시
            if (!A->IsActive() || !B->IsActive())
                continue;

            // (b) 같은 게임 오브젝트 (또는 부모-자식 관계)면 무시
            CGameObject* ownerA = A->GetOwner();
            CGameObject* ownerB = B->GetOwner();
            if (ownerA == ownerB
                || ownerA->GetParent() == ownerB
                || ownerB->GetParent() == ownerA)
                continue;

            // 5) 실제 충돌 여부 체크
            if (!A->Intersects(B))
                continue;

            // 6) **소유자 단위로 중복 처리 방지**
            //    (ownerA, ownerB) 쌍 정렬 => (min, max) 형태로 key 생성
            pair<CGameObject*, CGameObject*> ownerPair =
                ownerA < ownerB
                ? make_pair(ownerA, ownerB)
                : make_pair(ownerB, ownerA);

            // (a) 만약 이미 같은 ownerPair를 처리했다면, 이번 콜라이더 쌍은 넘긴다
            if (ownerCollided.count(ownerPair))
            {
                // 이미 소유자 단위로 충돌 처리된 적이 있어서 여기서는 Skip
                continue;
            }

            // (b) 아직 처리되지 않은 소유자 쌍이라면,
            //     이번 프레임 ownerPair를 처리 목록에 추가하고,
            //     실제 콜라이더 단위 로직도 수행
            ownerCollided.insert(ownerPair);
            currColliders.insert({ A, B });

            if (!m_vCollisions.count({ A, B }))
            {
                // 여기는 A,B가 이번 프레임 처음 충돌한 경우(Enter)
                A->NotifyEnter(B);
                B->NotifyEnter(A);
                A->SetCollision(true);
                B->SetCollision(true);

                // 우선순위 비교하여 겹침 해소 (단, HITBOX이면 해소하지 않음)
                CCollider* pusher = A;
                CCollider* pushed = B;
                if (A->GetPriority() > B->GetPriority())
                {
                    pusher = A;
                    pushed = B;
                }
                else if (B->GetPriority() > A->GetPriority())
                {
                    pusher = B;
                    pushed = A;
                }

                if (pusher->GetType() != ColliderType::HITBOX
                    && pushed->GetType() != ColliderType::HITBOX)
                {
                    ResolvePenetrationXZ(pusher, pushed);
                }
            }
            else
            {
                // 이미 충돌 중(STAY)
                A->NotifyStay(B, fTimeDelta);
                B->NotifyStay(A, fTimeDelta);
                A->SetCollision(true);
                B->SetCollision(true);

                CCollider* pusher = A;
                CCollider* pushed = B;
                if (A->GetPriority() > B->GetPriority())
                {
                    pusher = A;
                    pushed = B;
                }
                else if (B->GetPriority() > A->GetPriority())
                {
                    pusher = B;
                    pushed = A;
                }

                if (pusher->GetType() != ColliderType::HITBOX
                    && pushed->GetType() != ColliderType::HITBOX)
                {
                    ResolvePenetrationXZ(pusher, pushed);
                }
            }
        } 
    } 


    for (auto& pairPrev : m_vCollisions)
    {
        if (!currColliders.count(pairPrev))
        {
            pairPrev.first->NotifyExit(pairPrev.second);
            pairPrev.second->NotifyExit(pairPrev.first);
            pairPrev.first->SetCollision(false);
            pairPrev.second->SetCollision(false);
        }
    }

    m_vCollisions = move(currColliders);
}

void CCollisionMag::DebugDraw()
{
	for (auto& collider : m_vColliders)
	{
		collider->DebugDraw();
	}
}

void CCollisionMag::ResolvePenetrationXZ(CCollider* A, CCollider* B)
{
    if (auto capA = dynamic_cast<CCapsuleCollider*>(A))
    {
        if (auto capB = dynamic_cast<CCapsuleCollider*>(B))
        {
            // 두 캡슐 축 사이 최단점 계산
            XMVECTOR closestA, closestB;
            ClosestPointsSegmentSegment(
                capA->m_Capsule.A, capA->m_Capsule.B,
                capB->m_Capsule.A, capB->m_Capsule.B,
                closestA, closestB);

            float dist = XMVectorGetX(XMVector3Length(closestB - closestA));
            float rSum = capA->m_Capsule.Radius + capB->m_Capsule.Radius;
            float penetration = rSum - dist;
            if (penetration > 0.0f)
            {
                // 수평( XZ ) 노멀만 사용 → Y축 성분 0으로
                XMVECTOR raw = closestB - closestA;
                XMVECTOR hor = XMVectorSet(
                    XMVectorGetX(raw),
                    0.0f,
                    XMVectorGetZ(raw),
                    0.0f
                );
                float len = XMVectorGetX(XMVector3Length(hor));
                if (len < 1e-6f)
                {
                    hor = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
                }
                else
                {
                    hor = XMVector3Normalize(hor);
                }

                const float epsilon = 0.001f;
                XMVECTOR push = hor * (penetration + epsilon);

                // B(피격자)만 밀어내기
                CGameObject* goB = capB->GetOwner();
                XMVECTOR posB = goB->GetTransform()->Get_State(STATE::POSITION);
                goB->GetTransform()->Set_State(STATE::POSITION, posB + push);
            }
            return;
        }
    }

    //  A가 Capsule, B가 Sphere
    if (auto capA = dynamic_cast<CCapsuleCollider*>(A))
    {
        if (auto sphB = dynamic_cast<CSphereCollider*>(B))
        {
            // 캡슐 축 위의 최단점 (Sphere.Center → 캡슐)
            XMVECTOR centerS = XMLoadFloat3(&sphB->Sphere.Center);
            XMVECTOR c1 = ClosestPointOnSegment(centerS, capA->m_Capsule.A, capA->m_Capsule.B);

            float dist = XMVectorGetX(XMVector3Length(centerS - c1));
            float rSum = capA->m_Capsule.Radius + sphB->Sphere.Radius;
            float penetration = rSum - dist;
            if (penetration > 0.0f)
            {
                // 수평 노멀 ( Y=0 으로 고정 )
                XMVECTOR raw = centerS - c1;
                XMVECTOR hor = XMVectorSet(
                    XMVectorGetX(raw),
                    0.0f,
                    XMVectorGetZ(raw),
                    0.0f
                );
                float len = XMVectorGetX(XMVector3Length(hor));
                if (len < 1e-6f)
                {
                    hor = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
                }
                else
                {
                    hor = XMVector3Normalize(hor);
                }

                const float epsilon = 0.001f;
                XMVECTOR push = hor * (penetration + epsilon);

                // B(Sphere 피격자)만 밀어내기
                CGameObject* goB = sphB->GetOwner();
                XMVECTOR posB = goB->GetTransform()->Get_State(STATE::POSITION);
                goB->GetTransform()->Set_State(STATE::POSITION, posB + push);
            }
            return;
        }
    }

    //  A가 Capsule, B가 Box
    if (auto capA = dynamic_cast<CCapsuleCollider*>(A))
    {
        if (auto boxB = dynamic_cast<CBoxCollider*>(B))
        {
            // 캡슐 축 위의 최단점 (Box ↔ Segment)
            // → Box를 OBB로 보고, 캡슐 축 위의 각 점을 Box 로컬 → 최소 거리 판정
            // 우선 캡슐 축 중앙을 기준으로 처리 (약식)
            // 좀 더 정교하게 하려면 “선분 vs OBB 최단점 계산” 함수 필요

            // (간단 구현) 캡슐 축 A→B의 중점을 Pmid 로 삼아, 
            //   BoundingOrientedBox::ClosestPoint( Pmid ) 를 구한 뒤 처리

            XMVECTOR midA = 0.5f * (capA->m_Capsule.A + capA->m_Capsule.B);
            BoundingOrientedBox obb = boxB->Box;

            // ① Box 중심과 캡슐 축점 중 가장 가까운 축점(여기서는 중점)으로 계산
            //    DirectXCollision에 ClosestPoint 기능이 없으므로, 
            //    우리는 일일이 “선분 vs OBB” 최단점을 구하는 대신,
            //    Box.Center 와 midA → 둘 사이 수평 거리로만 처리
            XMVECTOR centerB = XMLoadFloat3(&boxB->Box.Center);

            XMVECTOR raw = midA - centerB;
            XMVECTOR hor = XMVectorSet(
                XMVectorGetX(raw),
                0.0f,
                XMVectorGetZ(raw),
                0.0f
            );
            float dist = XMVectorGetX(XMVector3Length(hor));
            float rSum = capA->m_Capsule.Radius + max( boxB->Box.Extents.x, boxB->Box.Extents.z );
            // 대략 Extents의 최대치로 반지름처럼 가정
            float penetration = rSum - dist;
            if (penetration > 0.0f)
            {
                if (dist < 1e-6f)
                {
                    hor = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
                }
                else
                {
                    hor = XMVector3Normalize(hor);
                }
                const float epsilon = 0.001f;
                XMVECTOR push = hor * (penetration + epsilon);

                // B(Box 피격자)만 밀어내기
                CGameObject* goB = boxB->GetOwner();
                XMVECTOR posB = goB->GetTransform()->Get_State(STATE::POSITION);
                goB->GetTransform()->Set_State(STATE::POSITION, posB + push);
            }
            return;
        }
    }

    //  A가 Sphere, B가 Sphere
    if (auto sphA = dynamic_cast<CSphereCollider*>(A))
    {
        if (auto sphB = dynamic_cast<CSphereCollider*>(B))
        {
            XMVECTOR centerA = XMLoadFloat3(&sphA->Sphere.Center);
            XMVECTOR centerB = XMLoadFloat3(&sphB->Sphere.Center);
            XMVECTOR raw = centerB - centerA;
            XMVECTOR hor = XMVectorSet(
                XMVectorGetX(raw),
                0.0f,
                XMVectorGetZ(raw),
                0.0f
            );
            float dist = XMVectorGetX(XMVector3Length(hor));
            float rSum = sphA->Sphere.Radius + sphB->Sphere.Radius;
            float penetration = rSum - dist;
            if (penetration > 0.0f)
            {
                if (dist < 1e-6f)
                {
                    hor = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
                }
                else
                {
                    hor = XMVector3Normalize(hor);
                }
                const float epsilon = 0.001f;
                XMVECTOR push = hor * (penetration + epsilon);

                // B(Sphere 피격자)만 밀어내기
                CGameObject* goB = sphB->GetOwner();
                XMVECTOR posB = goB->GetTransform()->Get_State(STATE::POSITION);
                goB->GetTransform()->Set_State(STATE::POSITION, posB + push);
            }
            return;
        }
    }

    //  A가 Sphere, B가 Capsule 또는 Box (역방향 처리)
    if (auto sphA = dynamic_cast<CSphereCollider*>(A))
    {
        if (auto capB = dynamic_cast<CCapsuleCollider*>(B))
        {
            // “구 vs 캡슐” 처리 → 캡슐이 피격자가 됨
            XMVECTOR centerS = XMLoadFloat3(&sphA->Sphere.Center);
            XMVECTOR c1 = ClosestPointOnSegment(centerS, capB->m_Capsule.A, capB->m_Capsule.B);

            float dist = XMVectorGetX(XMVector3Length(centerS - c1));
            float rSum = sphA->Sphere.Radius + capB->m_Capsule.Radius;
            float penetration = rSum - dist;
            if (penetration > 0.0f)
            {
                XMVECTOR raw = centerS - c1;
                XMVECTOR hor = XMVectorSet(
                    XMVectorGetX(raw),
                    0.0f,
                    XMVectorGetZ(raw),
                    0.0f
                );
                float len = XMVectorGetX(XMVector3Length(hor));
                if (len < 1e-6f)
                {
                    hor = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
                }
                else
                {
                    hor = XMVector3Normalize(hor);
                }
                const float epsilon = 0.001f;
                XMVECTOR push = hor * (penetration + epsilon);

                // B(Capsule 피격자)만 밀어내기
               CGameObject* goB = capB->GetOwner();
                XMVECTOR posB = goB->GetTransform()->Get_State(STATE::POSITION);
                float length = XMVectorGetX(XMVector3Length(push));  // push 벡터의 길이
                if (length > 0.5f)
                {
                    float s = 0.5f / length;   // 목표 길이/현재 길이
                    push = push * s;           // 방향 그대로, 크기만 줄임
                }
                goB->GetTransform()->Set_State(STATE::POSITION, posB - push);
            }
            return;
        }
        if (auto boxB = dynamic_cast<CBoxCollider*>(B))
        {
            // “구 vs 박스” → 박스가 피격자
            XMVECTOR centerS = XMLoadFloat3(&sphA->Sphere.Center);
            XMVECTOR centerB = XMLoadFloat3(&boxB->Box.Center);

            XMVECTOR raw = centerS - centerB;
            XMVECTOR hor = XMVectorSet(
                XMVectorGetX(raw),
                0.0f,
                XMVectorGetZ(raw),
                0.0f
            );
            float dist = XMVectorGetX(XMVector3Length(hor));
            float rSum = sphA->Sphere.Radius + max( boxB->Box.Extents.x, boxB->Box.Extents.z );
            float penetration = rSum - dist;
            if (penetration > 0.0f)
            {
                if (dist < 1e-6f)
                {
                    hor = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
                }
                else
                {
                    hor = XMVector3Normalize(hor);
                }
                const float epsilon = 0.001f;
                XMVECTOR push = hor * (penetration + epsilon);

                // B(Box 피격자)만 밀어내기
                CGameObject* goB = boxB->GetOwner();
                XMVECTOR posB = goB->GetTransform()->Get_State(STATE::POSITION);
                goB->GetTransform()->Set_State(STATE::POSITION, posB + push);
            }
            return;
        }
    }

    //  A가 Box, B가 Box
    if (auto boxA = dynamic_cast<CBoxCollider*>(A))
    {
        if (auto boxB = dynamic_cast<CBoxCollider*>(B))
        {
            // “박스 vs 박스” → 두 OBB 사이 최단점 계산 필요
            // 여기서는 간단하게 “중심점 기준 XZ 거리”로만 처리 (정확히는 OBB vs OBB 최소 거리 구해야 함)
            XMVECTOR centerA = XMLoadFloat3(&boxA->Box.Center);
            XMVECTOR centerB = XMLoadFloat3(&boxB->Box.Center);

            XMVECTOR raw = centerB - centerA;
            XMVECTOR hor = XMVectorSet(
                XMVectorGetX(raw),
                0.0f,
                XMVectorGetZ(raw),
                0.0f
            );
            float dist = XMVectorGetX(XMVector3Length(hor));
            float rA = max( boxA->Box.Extents.x, boxA->Box.Extents.z );
            float rB = max( boxB->Box.Extents.x, boxB->Box.Extents.z );
            float penetration = (rA + rB) - dist;
            if (penetration > 0.0f)
            {
                if (dist < 1e-6f)
                {
                    hor = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
                }
                else
                {
                    hor = XMVector3Normalize(hor);
                }
                const float epsilon = 0.001f;
                XMVECTOR push = hor * (penetration + epsilon);

                // B(Box 피격자)만 밀어내기
                CGameObject* goB = boxB->GetOwner();
                XMVECTOR posB = goB->GetTransform()->Get_State(STATE::POSITION);
                goB->GetTransform()->Set_State(STATE::POSITION, posB + push);
            }
            return;
        }
    }
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
