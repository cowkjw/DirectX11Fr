#include "PhysXMag.h"
#include "PhysXCollider.h"

HRESULT CPhysXMag::Initialize(_uint numThreads)
{
	// 1) Foundation 생성
	m_Foundation = PxCreateFoundation(
		PX_PHYSICS_VERSION,
		m_Allocator,
		m_ErrorCallback
	);
	if (!m_Foundation)
		return E_FAIL;

	// 2) Physics 객체 생성 (CPU only, 프로파일링 존 없음)
	m_Physics = PxCreatePhysics(
		PX_PHYSICS_VERSION,
		*m_Foundation,
		PxTolerancesScale(),
		/*trackOutstandingAllocs=*/true,
		/*profileZone=*/nullptr
	);
	if (!m_Physics)
		return E_FAIL;

	// 3) CPU 디스패처 생성 (numThreads 개수만큼 워커 스레드)
	m_Dispatcher = PxDefaultCpuDispatcherCreate(static_cast<PxU32>(numThreads));
	if (!m_Dispatcher)
		return E_FAIL;

	// 4) Scene 생성
	PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
	sceneDesc.cpuDispatcher = m_Dispatcher;                    // CPU 워커 연결
	sceneDesc.filterShader = PxDefaultSimulationFilterShader; // 기본 충돌 필터

	sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;
	m_Scene = m_Physics->createScene(sceneDesc);
	if (!m_Scene)
		return E_FAIL;

	if (auto* pvdClient = m_Scene->getScenePvdClient())
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	// (추가) 디버그 시각화 파라미터들
	m_Scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
	m_Scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1.0f);
	m_Scene->setVisualizationParameter(PxVisualizationParameter::eCONTACT_NORMAL, 1.0f);

	// 5) 기본 머티리얼 생성
	m_DefaultMaterial = m_Physics->createMaterial(
		/*staticFriction=*/0.5f,
		/*dynamicFriction=*/0.5f,
		/*restitution=*/0.6f
	);
	if (!m_DefaultMaterial)
		return E_FAIL;

	return S_OK;
}

void CPhysXMag::ReleasePhysX()
{
	if (m_Scene)
	{
		m_Scene->release();
		m_Scene = nullptr;
	}
	if (m_Dispatcher)
	{
		m_Dispatcher->release();
		m_Dispatcher = nullptr;
	}
	if (m_DefaultMaterial)
	{
		m_DefaultMaterial->release();
		m_DefaultMaterial = nullptr;
	}
	if (m_Physics)
	{
		m_Physics->release();
		m_Physics = nullptr;
	}
	if (m_Foundation)
	{
		m_Foundation->release();
		m_Foundation = nullptr;
	}
}

void CPhysXMag::Update(_float fTimeDelta)
{
	if (m_Scene)
	{
		m_Scene->simulate(fTimeDelta); // 시뮬레이션
		m_Scene->fetchResults(true);   // 결과 가져오기
	}
	for (auto& pCol : m_Colliders)
	{
		if (pCol)
			pCol->Update(fTimeDelta);
	}
}

PxRigidStatic* CPhysXMag::CreateRigidStatic(const PxTransform& transform)
{
	PxRigidStatic* actor = m_Physics->createRigidStatic(transform);
	if (actor)
		m_Scene->addActor(*actor);    // 씬에 등록
	return actor;
}

PxRigidDynamic* CPhysXMag::CreateRigidDynamic(const PxTransform& transform)
{
	PxRigidDynamic* actor = m_Physics->createRigidDynamic(transform);
	if (actor)
		m_Scene->addActor(*actor);    // 씬에 등록
	return actor;
}

void CPhysXMag::RegisterCollider(CPhysXCollider* pCol)
{
	if (pCol)
	{
		m_Colliders.push_back(pCol);
	}
	else
	{
		MSG_BOX("Failed to Register Collider");
	}
}

void CPhysXMag::UnregisterCollider(CPhysXCollider* pCol)
{
	auto it = remove(m_Colliders.begin(), m_Colliders.end(), pCol);
	m_Colliders.erase(it, m_Colliders.end());
}

CPhysXMag* CPhysXMag::Create(_uint numThreads)
{
	CPhysXMag* pInstance = new CPhysXMag();

	if (FAILED(pInstance->Initialize(numThreads)))
	{
		MSG_BOX("Failed to Created : CPhysXMag");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPhysXMag::Free()
{
	ReleasePhysX();
	__super::Free();
}
