#pragma once
#include "Camera.h"
#include "Client_Defines.h"

BEGIN_NAMESPACE(Engine)
class CTransform;
class CGameObject;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CThirdPersonCamera : public CCamera
{
public:
	typedef struct tagCameraFreeDesc : public CCamera::CAMERA_DESC
	{
		_float			fSmoth = {};
		CGameObject* pTarget = nullptr; // Target
	}THRIDCAMERA_DESC;

private:
	CThirdPersonCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CThirdPersonCamera(const CThirdPersonCamera& Prototype);
	virtual ~CThirdPersonCamera() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;

public:
	void SetTarget(class CGameObject* pTarget) { m_pTarget = pTarget; }

private:
	void InitializeCameraPosition();

private:
	_float			m_fSmooth = { }; // 카메라 움직임 스무스정도
	_float3 		m_vOffset = { 0.f, 0.f, 0.f };
	CGameObject* m_pTarget = nullptr; // Target
	CGameObject* m_pLockOnTarget = nullptr; // Target
	CTransform* m_pTargetTransform = nullptr; // Target Transform


	_float m_fFixedHeight = 15.f;      // 카메라 고정 높이
	_float m_fSingleCameraFixedHeight = 35.f;      // 카메라 고정 높이
	_float m_fDeadZoneWidth = 30.f;    // 좌우 데드존 크기
	_float m_fDeadZoneHeight = 10.0f;   // 상하 데드존 크기
	_float m_fMinCameraDistance = 30.0f; // 최소 카메라 거리
	_float m_fDistanceMultiplier = 0.4f; // 거리 배수

	_vector  m_lastMoveDir = XMVectorSet(0, 0, 1, 0);    // 초기 뒤로보기 기준: +Z 방향
	_vector  m_lastPlayerPos = XMVectorZero();          // 이전 프레임 플레이어 위치

	float m_cameraYaw = XMConvertToRadians(0.f);   // 카메라를 바라보는 수평 각도 (세계 축 기준)
	float m_cameraPitch = XMConvertToRadians(20.f);  // 카메라를 내려다보는 각도 (음수 = 아래)
	float m_cameraMinDist = 30.f;                       // 플레이어와 최소 거리
	float m_cameraMaxDist = 70.f;                       // 플레이어와 최대 거리
	float m_cameraSmooth = 5.f;                        // 보간 속도
	float m_cameraLookHeight =50.f;                        // 바라볼 때 플레이어 기준 높이

public:
	static CThirdPersonCamera* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

