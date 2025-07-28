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
	void TriggerShake(_float fDuration, _float fAmplitude)
	{
		m_bShaking = true;
		m_fShakeDuration = fDuration;
		m_fShakeTimeLeft = fDuration;
		m_fShakeAmplitude = fAmplitude;
	}

	void   OnHit(CGameObject* attacker, CGameObject* target, _vector offset, _float fDuration = 0.2f);
private:
	void InitializeCameraPosition();
	void UpdateShake(_float fTimeDelta,_vector& beginPos, _vector& outShakePos);
	void UpateTargetsCam(_float fTimeDelta);
	void UpateSingleTargetCam(_float fTimeDelta);
	void HitActionCamera(_float fTimeDelta);

private:
	_float			m_fSmooth = { }; // 카메라 움직임 스무스정도
	_float3 		m_vOffset = { 0.f, 0.f, 0.f };
	CGameObject*	m_pTarget = nullptr; // Target
	CGameObject*	m_pLockOnTarget = nullptr; // Target
	CTransform*		m_pTargetTransform = nullptr; // Target Transform


	_float m_fFixedHeight = 15.f;      // 카메라 고정 높이
	_float m_fDeadZoneWidth = 100.f;    // 좌우 데드존 크기
	_float m_fMinCameraDistance = 20.0f; // 최소 카메라 거리
	_float m_fDistanceMultiplier = 0.4f; // 거리 배수

	_vector  m_lastMoveDir = XMVectorSet(0, 0, 1, 0);    // 초기 뒤로보기 기준: +Z 방향
	_vector  m_lastPlayerPos = XMVectorZero();          // 이전 프레임 플레이어 위치

	_float m_cameraSmooth = 7.f;                        // 보간 속도
	_float m_cameraLookDistance = 80.f;                   

	// 카메라 쉐이킹
	_bool   m_bShaking = false;    // 쉐이크 중 플래그
	_float  m_fShakeDuration = 0.f;      // 전체 지속 시간
	_float  m_fShakeTimeLeft = 0.f;      // 남은 시간
	_float  m_fShakeAmplitude = 0.f;      // 최대 흔들림 크기


	// 전투할 때 타격감
	_bool    m_bHitCam = false;          // 타격 모드 활성화 플래그
	_float   m_fHitTime = 0.f;           // 타격 모드 경과 시간
	_float   m_fHitDuration = 0.2f;      // 타격 모드 지속 시간 
	_vector m_vHitOffset{};              // 타격 시 카메라 오프셋
	CGameObject* m_pHitAttacker = nullptr;
	CGameObject* m_pHitTarget = nullptr;


	_vector m_vLastPlayerPos{};     // 이전 프레임 플레이어 위치
	_vector m_vMoveDir = XMVectorSet(0, 0, 1, 0); // 초기 뒤쪽 방향

public:
	static CThirdPersonCamera* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

