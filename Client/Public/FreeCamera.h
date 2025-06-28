#pragma once
#include "Camera.h"
#include "Client_Defines.h"

BEGIN_NAMESPACE(Client)
class CFreeCamera final : public CCamera
{
public:
	typedef struct tagCameraFreeDesc : public CCamera::CAMERA_DESC
	{
		_float			fMouseSensor = {};
	}CAMERA_FREE_DESC;

private:
	CFreeCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFreeCamera(const CFreeCamera& Prototype);
	virtual ~CFreeCamera() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;

public:
	void SetMainpulate(_bool bMainpulate)
	{
		m_bMainpulate = bMainpulate;
	}
	_bool IsMainpulate() const
	{
		return m_bMainpulate;
	}
	void SetMouseSensor(_float fMouseSensor)
	{
		m_fMouseSensor = fMouseSensor;
	}
	_float GetMouseSensor() const
	{
		return m_fMouseSensor;
	}

private:
	_float			m_fMouseSensor = { };
	_bool			m_bMainpulate = true; // 카메라 조작 중인지 여부

public:
	static CFreeCamera* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

