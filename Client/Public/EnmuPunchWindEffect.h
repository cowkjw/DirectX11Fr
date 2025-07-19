#pragma once
#include "MeshEffect.h"

BEGIN_NAMESPACE(Client)
class CEnmuPunchWindEffect : public CMeshEffect
{
private:
	CEnmuPunchWindEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnmuPunchWindEffect(const CEnmuPunchWindEffect& Prototype);
	virtual ~CEnmuPunchWindEffect() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void SetColor(const _float4& vColor)
	{
		m_vColor = vColor;
	}

	void SetDirection(_bool bLeft)
	{
		m_bLeft = bLeft;
	}
	void UpdateTransform();

private:
	HRESULT Ready_Components();
	virtual HRESULT Bind_Shader() override;
	virtual void OnDisable() override;
	virtual void OnEnable() override;

private:

	_float4 m_vColor = { 1.f, 1.f, 1.f, 1.f };
	_float m_fRotationSpeed = 4.f;
	_float m_fDuration = 1.0f; // 지속 시간
	_bool m_bLeft = false;

public:
	static CEnmuPunchWindEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

