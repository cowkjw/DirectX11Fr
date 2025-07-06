#pragma once
#include "MeshEffect.h"

BEGIN_NAMESPACE(Client)
class CTanMigRingEffect : public CMeshEffect
{
private:
	CTanMigRingEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTanMigRingEffect(const CTanMigRingEffect& Prototype);
	virtual ~CTanMigRingEffect() = default;

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

	void SetDirection(_float fDir)
	{
		m_fDir = fDir;
	}

private:
	HRESULT Ready_Components();
	virtual HRESULT Bind_Shader() override;
	virtual void OnDisable() override;
	virtual void OnEnable() override;

private:
	_float m_fDir = 1.f; // 방향
	_float4 m_vColor = { 1.f, 1.f, 1.f, 1.f };
	_float m_fRotationSpeed = 0.7f;
	_float m_fScale = 15.f; // 크기
	_float m_fDestScale = 20.f; // 크기

public:
	static CTanMigRingEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

