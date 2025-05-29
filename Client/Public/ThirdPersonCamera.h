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
	_float			m_fSmooth = { }; // 카메라 움직임 스무스정도
	_float3 		m_vOffset = { 0.f, 0.f, 0.f };
	CGameObject* m_pTarget = nullptr; // Target
	CGameObject* m_pLockOnTarget = nullptr; // Target
	CTransform* m_pTargetTransform = nullptr; // Target Transform

public:
	static CThirdPersonCamera* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

