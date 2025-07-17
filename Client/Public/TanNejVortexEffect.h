#pragma once
#include "MeshEffect.h"

BEGIN_NAMESPACE(Client)
class CTanNejVortexEffect : public CMeshEffect
{
private:
	CTanNejVortexEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTanNejVortexEffect(const CTanNejVortexEffect& Prototype);
	virtual ~CTanNejVortexEffect() = default;

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

private:
	HRESULT Ready_Components();
	virtual HRESULT Bind_Shader() override;
	virtual void OnDisable() override;
	virtual void OnEnable() override;
	HRESULT Bind_Textures(_uint iMeshIndex);
	HRESULT Ready_Textures();
	void RiseVortex();
private:

	_float4 m_vColor = { 1.f, 1.f, 1.f, 1.f };
	_float m_fRotationSpeed = 4.f;
	_float m_fDuration = 1.0f; // 지속 시간
	array<_int, 3> m_iShaerderPasses = { 15,16,17 };
	array<CTexture*, TEX_MAX> m_Vortex3Textures{ nullptr, };
	array<CTexture*, TEX_MAX> m_Vortex4Textures{ nullptr, };


public:
	static CTanNejVortexEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

