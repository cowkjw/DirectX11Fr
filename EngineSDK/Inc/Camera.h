#pragma once
#include "GameObject.h"
class ENGINE_DLL CCamera : public CGameObject
{
public:
	typedef struct CameraDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float fFov {};
		_float fNear{};
		_float fFar {};
		_float3 vEye{};
		_float3 vAt{};
	}CAMERA_DESC;
protected:
	CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera(const CCamera& Prototype);
	virtual ~CCamera() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;	
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;

protected:
	/* 투영변환행렬을 만들어내기위한 정보들. */
	_float				m_fFov = {};
	_float				m_fNear = {};
	_float				m_fFar = {};
	_float				m_fAspect = {};

public:
	HRESULT Update_Camera();
public:
	virtual CGameObject* Clone(void* pArg) PURE;
	virtual void Free() override;
};

