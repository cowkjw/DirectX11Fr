#include "PhysXCollider.h"
#include "GameObject.h"
#include "GameInstance.h"

CPhysXCollider::CPhysXCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent(pDevice,pContext)
{
}

CPhysXCollider::CPhysXCollider(const CPhysXCollider& Prototype)
	:CComponent(Prototype),
	m_pPhysics(Prototype.m_pPhysics),
	m_pShape(Prototype.m_pShape),
	m_pActor(Prototype.m_pActor),
	m_bIsTrigger(Prototype.m_bIsTrigger),
	m_pMaterial(Prototype.m_pMaterial),
	m_bDebugDrawEnabled(Prototype.m_bDebugDrawEnabled),
	m_vLocalOffset(Prototype.m_vLocalOffset)
{
}

void CPhysXCollider::Update(_float fTimeDelta)
{
	if (!m_pActor || !m_pOwner) return;

	//PxTransform → 게임엔진 Transform 타입으로 변환
	auto trans = m_pOwner->GetTransform();
	if (!trans)
		return;

	// 2) DirectX XMVECTOR → PhysX PxTransform 변환
	XMVECTOR dxPos = trans->Get_State(STATE::POSITION);
//	XMVECTOR dxRot = trans->Get_State(STATE::ROTATION);  // 쿼터니언

	PxVec3 pxPos{ XMVectorGetX(dxPos),  XMVectorGetY(dxPos),  XMVectorGetZ(dxPos) };
	//PxQuat pxQuat{ XMVectorGetX(dxRot),  XMVectorGetY(dxRot),  XMVectorGetZ(dxRot),  XMVectorGetW(dxRot) };
	PxTransform newPose(pxPos);

	// 3) RigidDynamic이면 kinematic 모드로 따라오게, 아니라면 바로 덮어쓰기
	if (auto rd = m_pActor->is<PxRigidDynamic>())
	{
		// (초기화 때 eKINEMATIC 플래그를 켜두셨다면)
		rd->setKinematicTarget(newPose);
	}
	else
	{
		m_pActor->setGlobalPose(newPose);
	}

	PxTransform pose = m_pActor->getGlobalPose();    // PhysX 시뮬이 준 최신 위치
	XMVECTOR newPos = XMVectorSet(pose.p.x, pose.p.y, pose.p.z, 1.0f);
	trans->Set_State(STATE::POSITION, newPos);
//	// 2) PxTransform → XMVECTOR 변환
//	//    w 성분은 위치용이면 1.0f, 회전용 쿼터니언이면 pose.q.w 를 넣어줍니다.
//	XMVECTOR newPos = XMVectorSet(pose.p.x, pose.p.y, pose.p.z, 1.0f);
//	XMVECTOR newQuat = XMVectorSet(pose.q.x, pose.q.y, pose.q.z, pose.q.w);
//
//	// 3) 게임 오브젝트 Transform에 적용
//	trans->Set_State(STATE::POSITION, newPos);   // 위치
////	trans->Set_State(STATE::ROTATION, newQuat);  // 회전(쿼터니언)


//	auto trans = m_pOwner->GetTransform();
//	XMVECTOR posV = trans->Get_State(STATE::POSITION);
////	XMVECTOR rotQ = trans->Get_State(STATE::ROTATION);
//	PxVec3   pxPos{ XMVectorGetX(posV), XMVectorGetY(posV), XMVectorGetZ(posV) };
//	//PxQuat   pxRot{ XMVectorGetX(rotQ), XMVectorGetY(rotQ), XMVectorGetZ(rotQ), XMVectorGetW(rotQ) };
//	//PxTransform desiredPose(pxPos, pxRot);
//
//	//  - RigidDynamic을 키네매틱으로 설정했다면 setKinematicTarget
//	//if (auto rd = m_pActor->is<PxRigidDynamic>())
//	//{
//	//	rd->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
//	////	rd->setKinematicTarget(desiredPose);
//	//}
//	//else
//	//{
//	//	//  - StaticActor이거나 그냥 덮어쓰고 싶다면
//	//	//m_pActor->setGlobalPose(desiredPose);
//	//}
//
//	// 2) Actor → Transform (물리 시뮬 후 위치 반영; 필요 없으면 주석 처리)
//	PxTransform pose = m_pActor->getGlobalPose();
//	XMVECTOR newPos = XMVectorSet(pose.p.x, pose.p.y, pose.p.z, 1.0f);
//	XMVECTOR newQuat = XMVectorSet(pose.q.x, pose.q.y, pose.q.z, pose.q.w);
//
//	trans->Set_State(STATE::POSITION, newPos);
//	// trans->Set_State(STATE::ROTATION, newQuat);
}

HRESULT CPhysXCollider::Initialize_Prototype(PxPhysics* pPhysx, PxMaterial* pDefaultMat)
{
	m_pPhysics = pPhysx;
	m_pMaterial = pDefaultMat;

	return S_OK;
}

HRESULT CPhysXCollider::Initialize(void* pArg)
{
	PxRigidActor* actor = static_cast<PxRigidActor*>(pArg);
	if (!actor || !m_pShape) return E_FAIL;
	actor->attachShape(*m_pShape);
	m_pActor = actor;
	CGameInstance::Get_Instance()->GetScene()->addActor(*m_pActor);
	CGameInstance::Get_Instance()->RegisterCollider(this);
	m_pShape->setLocalPose(PxTransform(m_vLocalOffset));

	if (auto rd = m_pActor->is<PxRigidDynamic>())
	{
		rd->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
		//	rd->setKinematicTarget(desiredPose);
	}

	return S_OK;
}

void CPhysXCollider::DebugDraw()
{
	if (!m_bDebugDrawEnabled || !m_pOwner) return;
	auto* transform = m_pOwner->GetTransform();
	//Matrix world = transform->Get_WorldMatrix();
	//// Shape 유형별로 와이어프레임 드로우 호출 (예시)
	//// Box: 반쪽 Extents를 2배한 전체 Extents 사용
	//if (auto* box = dynamic_cast<CBoxCollider*>(this)) {
	//	Vector3 half = box->m_vHalfExtents;
	//	Renderer::DrawWireBox(world, half * 2.0f);  // 전체 Extents
	//}
	//// Sphere: 반지름 사용
	//if (auto* sphere = dynamic_cast<CSphereCollider*>(this)) {
	//	Renderer::DrawWireSphere(world, sphere->m_fRadius);
	//}
	//// Capsule: 반지름 및 높이 사용
	//if (auto* cap = dynamic_cast<CCapsuleCollider*>(this)) {
	//	Renderer::DrawWireCapsule(world, cap->m_fRadius, cap->m_fHalfHeight);
	//}
}

void CPhysXCollider::RenderInspector(IInspector& inspector)
{
	if (inspector.TreeNode("Collider Properties")) {
		_bool trig = m_bIsTrigger;
		if (inspector.Checkbox("Trigger", &trig)) SetTrigger(trig);
		_bool dbg = m_bDebugDrawEnabled;
		if (inspector.Checkbox("Debug Draw", &dbg)) SetDebugDrawEnabled(dbg);
		inspector.TreePop();
	}
}


json CPhysXCollider::Serialize()
{
	json j = CComponent::Serialize();
	j["localOffset"] = { m_vLocalOffset.x, m_vLocalOffset.y, m_vLocalOffset.z };
	j["isTrigger"] = m_bIsTrigger;

	return j;
}

void CPhysXCollider::Deserialize(const json& j)
{
	CComponent::Deserialize(j);
	if (j.contains("localOffset"))
	{
		auto& offset = j["localOffset"];
		m_vLocalOffset.x = offset[0];
		m_vLocalOffset.y = offset[1];
		m_vLocalOffset.z = offset[2];
	}
	if (j.contains("isTrigger"))
	{
		m_bIsTrigger = j["isTrigger"];
	}
}

CComponent* CPhysXCollider::Clone(void* pArg)
{
	CPhysXCollider* pInstance = new CPhysXCollider(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPhysXCollider");
		Safe_Release(pInstance);
	}

	return pInstance;

}

void CPhysXCollider::Free()
{
	__super::Free();
	//if (m_pShape&&!m_isCloned)
	//{
	//	m_pShape->release(); 
	//	m_pShape = nullptr;
	//}
}
