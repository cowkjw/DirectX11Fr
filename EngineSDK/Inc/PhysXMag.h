#pragma once
#include "Base.h"
BEGIN_NAMESPACE(Engine)
class CPhysXCollider;
class CPhysXMag final : public CBase 
{
private:
	CPhysXMag() = default;
	~CPhysXMag() = default;

public:
	HRESULT Initialize(_uint numThreads = 2);
	void ReleasePhysX();

	void Update(_float fTimeDelta);

	PxPhysics* GetPhysics() const { return m_Physics; }
	PxScene* GetScene() const { return m_Scene; }
	PxMaterial* GetDefaultMaterial() const { return m_DefaultMaterial; }

	PxRigidStatic* CreateRigidStatic(const PxTransform& transform);
	PxRigidDynamic* CreateRigidDynamic(const PxTransform& transform);

	void RegisterCollider(CPhysXCollider* pCol);
	void UnregisterCollider(CPhysXCollider* pCol);

private:
	 PxDefaultAllocator      m_Allocator;           // 기본 메모리 할당기
	 PxDefaultErrorCallback  m_ErrorCallback;       // 기본 에러 콜백
	 PxFoundation* m_Foundation = nullptr;
	 PxPhysics* m_Physics = nullptr;
	 PxDefaultCpuDispatcher* m_Dispatcher = nullptr;
	 PxCudaContextManager* m_CudaContextManager = nullptr;    // CUDA 컨텍스트 매니저
	 PxScene* m_Scene = nullptr;
	 PxMaterial* m_DefaultMaterial = nullptr;
	 vector<CPhysXCollider*> m_Colliders;

public:
	static CPhysXMag* Create(_uint numThreads = 2);
	virtual void Free() override;
};
END_NAMESPACE
