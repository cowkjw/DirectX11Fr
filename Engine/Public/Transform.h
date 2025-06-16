#pragma once

#include "Component.h"

BEGIN_NAMESPACE(Engine)
class CNavigation;

class ENGINE_DLL CTransform final : public CComponent
{
public:
	typedef struct tagTransformDesc
	{
		_float			fSpeedPerSec;
		_float			fRotationPerSec;
	}TRANSFORM_DESC;

protected:
	CTransform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTransform(const CTransform& Prototype);
	virtual ~CTransform() = default;

public:
	_vector Get_State(STATE eState) {
		return XMLoadFloat4x4(&m_WorldMatrix).r[ToIndex(eState)];
	}

	_float3 Get_Scaled();

	void Set_State(STATE eState, _fvector vState)
	{
		XMStoreFloat4(reinterpret_cast<_float4*>(&m_WorldMatrix.m[ToIndex(eState)]), vState);
	}

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);


public:
	void Scaling(_float fX = 1.f, _float fY = 1.f, _float fZ = 1.f);
	void Scaling(const _float3& vScale);
public:
	void Go_Straight(_float fTimeDelta, CNavigation* pNav = nullptr);
	void Go_Backward(_float fTimeDelta, CNavigation* pNav = nullptr);
	void Go_Right(_float fTimeDelta, CNavigation* pNav = nullptr);
	void Go_Left(_float fTimeDelta, CNavigation* pNav = nullptr);
	void MoveDirection(_fvector vDirection, _float fTimeDelta, CNavigation* pNav = nullptr);
	void Follow_Target(_fvector vTarget, _float fTimeDelta, _float fMinDistance);

	void Rotate_EulerAngles(const _float3& vEulerAngles);
	void Turn(_fvector vAxis, _float fTimeDelta);
	void RotateToDirection(_fvector dir);


	void FollowParent(CTransform* pParentTransform);


	json Serialize() override;
	void Deserialize(const json& j) override;

	void Set_Parent(CGameObject* pParent);

public:
	_matrix Get_WorldMatrix_Inverse() const
	{
		return XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix));
	}

	auto& Get_WorldMatrix()  { return m_WorldMatrix; } // float4x4

	const _float3& Get_EulerAngles() const { return m_vEulerAngles; }
	_vector Get_RotationQuaternion() const;
	void Set_RotationQuaternion(_fvector vQuat);

	void Set_WorldMatrix(_float4x4 worldMatrix) {
		m_bDirty = true;
		m_WorldMatrix = worldMatrix;
	}

	void LookAt(_fvector vAt);
	void LookAtXZ(_fvector vAt);


	_bool IsDirty() const { 
		return m_bDirty; }
	void SetDirty(_bool bDirty) { m_bDirty = bDirty; }
	void UpdateEulerAngles();

	_float Get_SpeedPerSec() const { return m_fSpeedPerSec; }
	_float Get_RotationPerSec() const { return m_fRotationPerSec; }

public:
	HRESULT Bind_ShaderResource(class CShader* pShader, const _char* pConstantName);

private:
	_float4x4				m_WorldMatrix = {};
	_float4x4				m_LocalMatrix = {};
	_float3					m_vEulerAngles{};
	_float					m_fSpeedPerSec = {};
	_float					m_fRotationPerSec = {};
	_bool					m_bDirty = false;


public:
	static CTransform* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END_NAMESPACE